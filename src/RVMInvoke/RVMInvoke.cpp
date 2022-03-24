#include "RVMInvoke.h"
#include <cpu_provider_factory.h>

Ort::Value create_tensor(const cv::Mat& mat,
	const std::vector<int64_t>& tensor_dims,
	const Ort::MemoryInfo& memory_info_handler,
	std::vector<float>& tensor_value_handler,
	CHW_OR_HWC data_format)
	throw(std::runtime_error)
{
	const unsigned int rows = mat.rows;
	const unsigned int cols = mat.cols;
	const unsigned int channels = mat.channels();

	cv::Mat mat_ref;
	if (mat.type() != CV_32FC(channels)) mat.convertTo(mat_ref, CV_32FC(channels));
	else mat_ref = mat; // reference only. zero-time cost. support 1/2/3/... channels

	if (tensor_dims.size() != 4) throw std::runtime_error("dims mismatch.");
	if (tensor_dims.at(0) != 1) throw std::runtime_error("batch != 1");

	// CXHXW
	if (data_format == CHW_OR_HWC::CHW)
	{

		const unsigned int target_channel = tensor_dims.at(1);
		const unsigned int target_height = tensor_dims.at(2);
		const unsigned int target_width = tensor_dims.at(3);
		const unsigned int target_tensor_size = target_channel * target_height * target_width;
		if (target_channel != channels) throw std::runtime_error("channel mismatch.");

		tensor_value_handler.resize(target_tensor_size);

		cv::Mat resize_mat_ref;
		if (target_height != rows || target_width != cols)
			cv::resize(mat_ref, resize_mat_ref, cv::Size(target_width, target_height));
		else resize_mat_ref = mat_ref; // reference only. zero-time cost.

		std::vector<cv::Mat> mat_channels;
		cv::split(resize_mat_ref, mat_channels);
		// CXHXW
		for (unsigned int i = 0; i < channels; ++i)
			std::memcpy(tensor_value_handler.data() + i * (target_height * target_width),
				mat_channels.at(i).data, target_height * target_width * sizeof(float));

		return Ort::Value::CreateTensor<float>(memory_info_handler, tensor_value_handler.data(),
			target_tensor_size, tensor_dims.data(),
			tensor_dims.size());

		//return Ort::Value::CreateTensor(memory_info_handler, tensor_value_handler.data(), tensor_value_handler.size() * sizeof(uint16_t), tensor_dims.data(), tensor_dims.size(), ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16);

	}

	// HXWXC
	const unsigned int target_channel = tensor_dims.at(3);
	const unsigned int target_height = tensor_dims.at(1);
	const unsigned int target_width = tensor_dims.at(2);
	const unsigned int target_tensor_size = target_channel * target_height * target_width;
	if (target_channel != channels) throw std::runtime_error("channel mismatch!");
	tensor_value_handler.resize(target_tensor_size);

	cv::Mat resize_mat_ref;
	if (target_height != rows || target_width != cols)
		cv::resize(mat_ref, resize_mat_ref, cv::Size(target_width, target_height));
	else resize_mat_ref = mat_ref; // reference only. zero-time cost.

	std::memcpy(tensor_value_handler.data(), resize_mat_ref.data, target_tensor_size * sizeof(float));

	return Ort::Value::CreateTensor<float>(memory_info_handler, tensor_value_handler.data(),
		target_tensor_size, tensor_dims.data(),
		tensor_dims.size());
	//return Ort::Value::CreateTensor(memory_info_handler, tensor_value_handler.data(), tensor_value_handler.size() * sizeof(uint16_t), tensor_dims.data(), tensor_dims.size(), ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16);

}

RobustVideoMatting::RobustVideoMatting(const std::string& _onnx_path, unsigned int _num_threads) :
	log_id(_onnx_path.data()), num_threads(_num_threads)
{
	auto to_wstring = [](const std::string& str) {
		unsigned len = str.size() * 2;
		setlocale(LC_CTYPE, "");
		wchar_t* p = new wchar_t[len];
		mbstowcs(p, str.c_str(), len);
		std::wstring wstr(p);
		delete[] p;
		return wstr;
	};
	std::wstring _w_onnx_path(to_wstring(_onnx_path));
	onnx_path = _w_onnx_path.data();
	ort_env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, log_id);
	// 0. session options
	Ort::SessionOptions session_options;
	session_options.SetIntraOpNumThreads(num_threads);
	session_options.SetGraphOptimizationLevel(
		GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
	session_options.SetLogSeverityLevel(4);


	try {
		Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_CPU(session_options, 0));
		ort_session = new Ort::Session(ort_env, onnx_path, session_options);
	}
	catch (const std::exception& e) {
		std::cerr << e.what();
		return;
	}
	// 1. session
	// GPU Compatibility.

	//OrtSessionOptionsAppendExecutionProvider_CUDA(session_options, 0); // C API stable.


#if LITEORT_DEBUG
	std::cout << "Load " << onnx_path << " done!" << std::endl;
#endif
}

RobustVideoMatting::~RobustVideoMatting()
{
	if (ort_session)
		delete ort_session;
	ort_session = nullptr;
}

int64_t RobustVideoMatting::value_size_of(const std::vector<int64_t>& dims)
{
	if (dims.empty()) return 0;
	int64_t value_size = 1;
	for (const auto& size : dims) value_size *= size;
	return value_size;
}

std::vector<Ort::Value> RobustVideoMatting::transform(const cv::Mat& mat)
{
	cv::Mat src = mat.clone();
	const unsigned int img_height = mat.rows;
	const unsigned int img_width = mat.cols;
	std::vector<int64_t>& src_dims = dynamic_input_node_dims.at(0); // (1,3,h,w)
	// update src height and width
	src_dims.at(2) = img_height;
	src_dims.at(3) = img_width;
	// assume that rxi's dims and value_handler was updated by last step in a while loop.
	std::vector<int64_t>& r1i_dims = dynamic_input_node_dims.at(1); // (1,?,?h,?w)
	std::vector<int64_t>& r2i_dims = dynamic_input_node_dims.at(2); // (1,?,?h,?w)
	std::vector<int64_t>& r3i_dims = dynamic_input_node_dims.at(3); // (1,?,?h,?w)
	std::vector<int64_t>& r4i_dims = dynamic_input_node_dims.at(4); // (1,?,?h,?w)
	std::vector<int64_t>& dsr_dims = dynamic_input_node_dims.at(5); // (1)
	int64_t src_value_size = this->value_size_of(src_dims); // (1*3*h*w)
	int64_t r1i_value_size = this->value_size_of(r1i_dims); // (1*?*?h*?w)
	int64_t r2i_value_size = this->value_size_of(r2i_dims); // (1*?*?h*?w)
	int64_t r3i_value_size = this->value_size_of(r3i_dims); // (1*?*?h*?w)
	int64_t r4i_value_size = this->value_size_of(r4i_dims); // (1*?*?h*?w)
	int64_t dsr_value_size = this->value_size_of(dsr_dims); // 1
	dynamic_src_value_handler.resize(src_value_size);

	// normalize & RGB
	cv::cvtColor(src, src, cv::COLOR_BGR2RGB); // (h,w,3)
	src.convertTo(src, CV_32FC3, 1.0f / 255.0f, 0.f); // 0.~1.



	// convert to tensor.
	std::vector<Ort::Value> input_tensors;
	input_tensors.emplace_back(create_tensor(
		src, src_dims, memory_info_handler, dynamic_src_value_handler,
		CHW_OR_HWC::CHW
	));


	input_tensors.emplace_back(Ort::Value::CreateTensor<float>(
		memory_info_handler, dynamic_r1i_value_handler.data(),
		r1i_value_size, r1i_dims.data(), r1i_dims.size()
		));
	input_tensors.emplace_back(Ort::Value::CreateTensor<float>(
		memory_info_handler, dynamic_r2i_value_handler.data(),
		r2i_value_size, r2i_dims.data(), r2i_dims.size()
		));
	input_tensors.emplace_back(Ort::Value::CreateTensor<float>(
		memory_info_handler, dynamic_r3i_value_handler.data(),
		r3i_value_size, r3i_dims.data(), r3i_dims.size()
		));
	input_tensors.emplace_back(Ort::Value::CreateTensor<float>(
		memory_info_handler, dynamic_r4i_value_handler.data(),
		r4i_value_size, r4i_dims.data(), r4i_dims.size()
		));
	input_tensors.emplace_back(Ort::Value::CreateTensor<float>(
		memory_info_handler, dynamic_dsr_value_handler.data(),
		dsr_value_size, dsr_dims.data(), dsr_dims.size()
		));

	return input_tensors;
}

void RobustVideoMatting::detect(const cv::Mat& mat, MattingContent& content,
	float downsample_ratio, cv::Scalar backgroundColor, bool video_mode)
{
	if (mat.empty()) return;
	// 0. set dsr at runtime.
	dynamic_dsr_value_handler.at(0) = downsample_ratio;

	// 1. make input tensors, src, rxi, dsr
	std::vector<Ort::Value> input_tensors = this->transform(mat);
	// 2. inference, fgr, pha, rxo.
	std::vector<Ort::Value> output_tensors;
	try {
		output_tensors = ort_session->Run(
			Ort::RunOptions{ nullptr }, input_node_names.data(),
			input_tensors.data(), num_inputs, output_node_names.data(),
			num_outputs
		);
	}
	catch (Ort::Exception& e) {
		std::cerr << e.what();
	}
	// 3. generate matting
	this->generate_matting(output_tensors, content, backgroundColor);
	// 4. update context (needed for video detection.)
	if (video_mode)
	{
		context_is_update = false; // init state.
		this->update_context(output_tensors);
	}

}


void RobustVideoMatting::detect_video(const std::string& video_path,
	const std::string& output_path,
	std::vector<MattingContent>& contents, cv::Size NetInputImgSize, float downsample_ratio, cv::Scalar backgroundColor,
	bool save_contents,
	unsigned int writer_fps)
{
	// 0. init video capture
	cv::VideoCapture video_capture(video_path);
	const unsigned int width = video_capture.get(cv::CAP_PROP_FRAME_WIDTH);
	const unsigned int height = video_capture.get(cv::CAP_PROP_FRAME_HEIGHT);
	const unsigned int frame_count = video_capture.get(cv::CAP_PROP_FRAME_COUNT);
	if (writer_fps == 0)// not setting fps, we consider it as the input video fps
	{
		writer_fps = video_capture.get(cv::CAP_PROP_FPS);
	}
	if (!video_capture.isOpened())
	{
		std::cout << "Can not open video: " << video_path << "\n";
		return;
	}
	// 1. init video writer
	cv::VideoWriter video_writer(output_path, cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
		writer_fps, cv::Size(width, height));
	if (!video_writer.isOpened())
	{
		std::cout << "Can not open writer: " << output_path << "\n";
		return;
	}

	// 2. matting loop

	unsigned int i = 0;
	cv::Mat mat;
	while (video_capture.read(mat))
	{
		auto ct1 = std::chrono::high_resolution_clock::now();
		auto t1 = std::chrono::time_point_cast<std::chrono::milliseconds>(ct1).time_since_epoch()
			.count();
		i += 1;

		cv::Mat resizedImg;
		cv::resize(mat, resizedImg, NetInputImgSize);

		MattingContent content;
		this->detect(resizedImg, content, downsample_ratio, backgroundColor, true); // video_mode true

		bool in = false;
		if (in) {
			//start of process mask
			cv::Mat backgroundMask(mat.size(), CV_8UC1, cv::Scalar(255));
			float threshold = 1;
			backgroundMask = content.pha_mat < threshold;

			backgroundMask.copyTo(backgroundMask);

			auto backgroundColor = cv::Scalar(0, 0, 0);


			// Resize the size of the mask back to the size of the original input.
			cv::resize(backgroundMask, backgroundMask, mat.size());

			float smoothContour = 0.2;
			// Smooth mask with a fast filter (box).
			if (smoothContour > 0.0) {
				int k_size = (int)(100 * smoothContour);
				cv::boxFilter(backgroundMask, backgroundMask, backgroundMask.depth(), cv::Size(k_size, k_size));
				backgroundMask = backgroundMask > 128;
			}

			// apply the mask as-is back onto the main image.
			mat.setTo(backgroundColor, backgroundMask);
			//end of process mask
		}
		else {
			cv::resize(content.merge_mat, mat, cv::Size(width, height));
		}

		// 3. save contents and writing out.
		if (content.flag)
		{
			if (save_contents) {
				contents.push_back(content);
			}
			if (!content.merge_mat.empty())
			{
				video_writer.write(mat);
			}
		}
		// 4. check context states.
		if (!context_is_update) break;

		auto ct2 = std::chrono::high_resolution_clock::now();
		auto t2 = std::chrono::time_point_cast<std::chrono::milliseconds>(ct2).time_since_epoch()
			.count();
		std::cout << "time consume is : " << t2 - t1 << "\n";

		std::cout << i << "/" << frame_count << " done!" << "\n";

	}

	// 5. release
	video_capture.release();
	video_writer.release();

}

void RobustVideoMatting::generate_matting(std::vector<Ort::Value>& output_tensors,
	MattingContent& content, cv::Scalar backgroundColor)
{
	Ort::Value& fgr = output_tensors.at(0); // fgr (1,3,h,w) 0.~1.
	Ort::Value& pha = output_tensors.at(1); // pha (1,1,h,w) 0.~1.
	auto fgr_dims = fgr.GetTypeInfo().GetTensorTypeAndShapeInfo().GetShape();
	auto pha_dims = pha.GetTypeInfo().GetTensorTypeAndShapeInfo().GetShape();
	const unsigned int height = fgr_dims.at(2); // output height
	const unsigned int width = fgr_dims.at(3); // output width
	const unsigned int channel_step = height * width;
	// fast assign & channel transpose(CHW->HWC).
	float* fgr_ptr = fgr.GetTensorMutableData<float>();
	float* pha_ptr = pha.GetTensorMutableData<float>();
	cv::Mat rmat(height, width, CV_32FC1, fgr_ptr);
	cv::Mat gmat(height, width, CV_32FC1, fgr_ptr + channel_step);
	cv::Mat bmat(height, width, CV_32FC1, fgr_ptr + 2 * channel_step);
	cv::Mat pmat(height, width, CV_32FC1, pha_ptr);
	rmat *= 255.;
	bmat *= 255.;
	gmat *= 255.;
	cv::Mat rest = 1. - pmat;
	cv::Mat mbmat = bmat.mul(pmat) + rest * backgroundColor.val[0];
	cv::Mat mgmat = gmat.mul(pmat) + rest * backgroundColor.val[1];
	cv::Mat mrmat = rmat.mul(pmat) + rest * backgroundColor.val[2];
	std::vector<cv::Mat> fgr_channel_mats, merge_channel_mats;
	fgr_channel_mats.push_back(bmat);
	fgr_channel_mats.push_back(gmat);
	fgr_channel_mats.push_back(rmat);
	merge_channel_mats.push_back(mbmat);
	merge_channel_mats.push_back(mgmat);
	merge_channel_mats.push_back(mrmat);

	content.pha_mat = pmat;
	cv::merge(fgr_channel_mats, content.fgr_mat);
	cv::merge(merge_channel_mats, content.merge_mat);
	content.fgr_mat.convertTo(content.fgr_mat, CV_8UC3);
	content.merge_mat.convertTo(content.merge_mat, CV_8UC3);

	// convert into 0-255 mat
	pmat *= 255.;
	content.pha_mat.convertTo(content.pha_mat, CV_8UC3);

	content.flag = true;
}


void RobustVideoMatting::update_context(std::vector<Ort::Value>& output_tensors)
{
	// 0. update context for video matting.
	Ort::Value& r1o = output_tensors.at(2); // fgr (1,?,?h,?w)
	Ort::Value& r2o = output_tensors.at(3); // pha (1,?,?h,?w)
	Ort::Value& r3o = output_tensors.at(4); // pha (1,?,?h,?w)
	Ort::Value& r4o = output_tensors.at(5); // pha (1,?,?h,?w)
	auto r1o_dims = r1o.GetTypeInfo().GetTensorTypeAndShapeInfo().GetShape();
	auto r2o_dims = r2o.GetTypeInfo().GetTensorTypeAndShapeInfo().GetShape();
	auto r3o_dims = r3o.GetTypeInfo().GetTensorTypeAndShapeInfo().GetShape();
	auto r4o_dims = r4o.GetTypeInfo().GetTensorTypeAndShapeInfo().GetShape();
	// 1. update rxi's shape according to last rxo
	dynamic_input_node_dims.at(1) = r1o_dims;
	dynamic_input_node_dims.at(2) = r2o_dims;
	dynamic_input_node_dims.at(3) = r3o_dims;
	dynamic_input_node_dims.at(4) = r4o_dims;
	// 2. update rxi's value according to last rxo
	int64_t new_r1i_value_size = this->value_size_of(r1o_dims); // (1*?*?h*?w)
	int64_t new_r2i_value_size = this->value_size_of(r2o_dims); // (1*?*?h*?w)
	int64_t new_r3i_value_size = this->value_size_of(r3o_dims); // (1*?*?h*?w)
	int64_t new_r4i_value_size = this->value_size_of(r4o_dims); // (1*?*?h*?w)
	dynamic_r1i_value_handler.resize(new_r1i_value_size);
	dynamic_r2i_value_handler.resize(new_r2i_value_size);
	dynamic_r3i_value_handler.resize(new_r3i_value_size);
	dynamic_r4i_value_handler.resize(new_r4i_value_size);
	float* new_r1i_value_ptr = r1o.GetTensorMutableData<float>();
	float* new_r2i_value_ptr = r2o.GetTensorMutableData<float>();
	float* new_r3i_value_ptr = r3o.GetTensorMutableData<float>();
	float* new_r4i_value_ptr = r4o.GetTensorMutableData<float>();
	std::memcpy(dynamic_r1i_value_handler.data(), new_r1i_value_ptr, new_r1i_value_size * sizeof(float));
	std::memcpy(dynamic_r2i_value_handler.data(), new_r2i_value_ptr, new_r2i_value_size * sizeof(float));
	std::memcpy(dynamic_r3i_value_handler.data(), new_r3i_value_ptr, new_r3i_value_size * sizeof(float));
	std::memcpy(dynamic_r4i_value_handler.data(), new_r4i_value_ptr, new_r4i_value_size * sizeof(float));
	context_is_update = true;
}