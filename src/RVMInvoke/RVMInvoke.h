#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "onnxruntime_cxx_api.h"
#include <opencv.hpp>

enum class CHW_OR_HWC
{
	CHW = 0, HWC = 1
};

typedef struct  MattingContentType
{
	cv::Mat fgr_mat; // fore ground mat 3 channel (R,G,B) 0.~1. or 0~255
	cv::Mat pha_mat; // alpha(matte) 0.~1. or 0~255
	cv::Mat merge_mat; // merge bg and fg according pha
	bool flag;

	MattingContentType() : flag(false)
	{};
} MattingContent;

class  RobustVideoMatting {
	Ort::Env ort_env;
	Ort::Session* ort_session = nullptr;
	// CPU MemoryInfo
	Ort::AllocatorWithDefaultOptions allocator;
	Ort::MemoryInfo memory_info_handler = Ort::MemoryInfo::CreateCpu(
		OrtArenaAllocator, OrtMemTypeDefault);
	// hardcode input node names
	unsigned int num_inputs = 6;
	std::vector<const char*> input_node_names = {
		"src",
		"r1i",
		"r2i",
		"r3i",
		"r4i",
		"downsample_ratio"
	};
	// init dynamic input dims
	std::vector<std::vector<int64_t>> dynamic_input_node_dims = {
		{1, 3, 1280, 720}, // src  (b=1,c,h,w)
		{1, 1, 1,    1}, // r1i
		{1, 1, 1,    1}, // r2i
		{1, 1, 1,    1}, // r3i
		{1, 1, 1,    1}, // r4i
		{1} // downsample_ratio dsr
	}; // (1, 16, ?h, ?w) for inner loop rxi

	// hardcode output node names
	unsigned int num_outputs = 6;
	std::vector<const char*> output_node_names = {
		"fgr",
		"pha",
		"r1o",
		"r2o",
		"r3o",
		"r4o"
	};
	const wchar_t* onnx_path = nullptr;
	const char* log_id = nullptr;

	// input values handler & init
	std::vector<float> dynamic_src_value_handler;
	std::vector<float> dynamic_r1i_value_handler = { 0.0f }; // init 0. with shape (1,1,1,1)
	std::vector<float> dynamic_r2i_value_handler = { 0.0f };
	std::vector<float> dynamic_r3i_value_handler = { 0.0f };
	std::vector<float> dynamic_r4i_value_handler = { 0.0f };
	std::vector<float> dynamic_dsr_value_handler = { 0.25f }; // downsample_ratio with shape (1)

protected:
	const unsigned int num_threads; // initialize at runtime.

public:
	explicit RobustVideoMatting(const std::string& _onnx_path, unsigned int _num_threads = 1);

	~RobustVideoMatting();

protected:
	RobustVideoMatting(const RobustVideoMatting&) = delete; //
	RobustVideoMatting(RobustVideoMatting&&) = delete; //
	RobustVideoMatting& operator=(const RobustVideoMatting&) = delete; //
	RobustVideoMatting& operator=(RobustVideoMatting&&) = delete; //

private:
	// return normalized src, rxi, dsr Tensors
	std::vector<Ort::Value> transform(const cv::Mat& mat);

	int64_t value_size_of(const std::vector<int64_t>& dims); // get value size

	void generate_matting(std::vector<Ort::Value>& output_tensors,
		MattingContent& content, cv::Scalar backgroundColor);

	void update_context(std::vector<Ort::Value>& output_tensors);

public:

	bool context_is_update = false;

	/**
	 * Image Matting Using RVM(https://github.com/PeterL1n/RobustVideoMatting)
	 * @param mat: cv::Mat BGR HWC
	 * @param content: types::MattingContent to catch the detected results.
	 * @param downsample_ratio: 0.25 by default.
	 * @param video_mode: false by default.
	 * See https://github.com/PeterL1n/RobustVideoMatting/blob/master/documentation/inference_zh_Hans.md
	 */
	void detect(const cv::Mat& mat, MattingContent& content,
		float downsample_ratio = 0.25f, cv::Scalar backgroundColor = { 153.,255. ,120. }, bool video_mode = false);
	/**
	 * Video Matting Using RVM(https://github.com/PeterL1n/RobustVideoMatting)
	 * @param video_path: eg. xxx/xxx/input.mp4
	 * @param output_path: eg. xxx/xxx/output.mp4
	 * @param contents: vector of MattingContent to catch the detected results.
	 * @param save_contents: false by default, whether to save MattingContent.
	 * @param downsample_ratio: 0.25 by default.
	 * See https://github.com/PeterL1n/RobustVideoMatting/blob/master/documentation/inference_zh_Hans.md
	 * @param writer_fps: FPS for VideoWriter, 20 by default.
	 */
	void detect_video(const std::string& video_path,
		const std::string& output_path,
		std::vector<MattingContent>& contents, cv::Size NetInputImgSize, float downsample_ratio = 0.25f, cv::Scalar backgroundColor = { 153.,255. ,120. },
		bool save_contents = false,
		unsigned int writer_fps = 0);

};