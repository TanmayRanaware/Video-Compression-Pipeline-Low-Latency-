cmake_minimum_required(VERSION 3.14)
project(telehealth_codec VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(TELECODEC_USE_FFMPEG "Use FFmpeg for video capture/demux" ON)
option(TELECODEC_BUILD_TESTS "Build unit tests" ON)
option(TELECODEC_BUILD_BENCHMARKS "Build benchmarks" ON)
option(TELECODEC_BUILD_APPS "Build CLI applications" ON)

# Include directory
set(TELECODEC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TELECODEC_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ========== Library: codec ==========
add_library(telehealth_codec STATIC
  ${TELECODEC_SRC_DIR}/codec/Frame.cpp
  ${TELECODEC_SRC_DIR}/codec/YuvConverter.cpp
  ${TELECODEC_SRC_DIR}/codec/Block.cpp
  ${TELECODEC_SRC_DIR}/codec/MotionEstimation.cpp
  ${TELECODEC_SRC_DIR}/codec/MotionCompensation.cpp
  ${TELECODEC_SRC_DIR}/codec/Residual.cpp
  ${TELECODEC_SRC_DIR}/codec/Transform.cpp
  ${TELECODEC_SRC_DIR}/codec/Quantizer.cpp
  ${TELECODEC_SRC_DIR}/codec/EntropyCoder.cpp
  ${TELECODEC_SRC_DIR}/codec/BitstreamWriter.cpp
  ${TELECODEC_SRC_DIR}/codec/RateControl.cpp
  ${TELECODEC_SRC_DIR}/codec/Encoder.cpp
)
target_include_directories(telehealth_codec PUBLIC ${TELECODEC_INCLUDE_DIR})

# ========== Library: pipeline ==========
add_library(telehealth_pipeline STATIC
  ${TELECODEC_SRC_DIR}/pipeline/Pipeline.cpp
  ${TELECODEC_SRC_DIR}/pipeline/Stage.cpp
)
target_include_directories(telehealth_pipeline PUBLIC ${TELECODEC_INCLUDE_DIR})
target_link_libraries(telehealth_pipeline PUBLIC telehealth_codec)

# ========== Library: io ==========
set(IO_SOURCES
  ${TELECODEC_SRC_DIR}/io/FileBitstreamSink.cpp
  ${TELECODEC_SRC_DIR}/io/FileYuvSink.cpp
  ${TELECODEC_SRC_DIR}/io/UdpPacketSink.cpp
  ${TELECODEC_SRC_DIR}/io/UdpReceiver.cpp
  ${TELECODEC_SRC_DIR}/io/JitterBuffer.cpp
  ${TELECODEC_SRC_DIR}/io/FfmpegVideoSource.cpp
)
add_library(telehealth_io STATIC ${IO_SOURCES})
target_include_directories(telehealth_io PUBLIC ${TELECODEC_INCLUDE_DIR})
target_link_libraries(telehealth_io PUBLIC telehealth_codec)
if(TELECODEC_USE_FFMPEG)
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(FFMPEG libavcodec libavformat libavutil libswscale)
    if(FFMPEG_FOUND)
      target_include_directories(telehealth_io PRIVATE ${FFMPEG_INCLUDE_DIRS})
      target_link_libraries(telehealth_io PRIVATE ${FFMPEG_LIBRARIES})
      target_compile_definitions(telehealth_io PRIVATE TELECODEC_USE_FFMPEG=1)
    endif()
  endif()
endif()

# ========== Library: util ==========
add_library(telehealth_util STATIC
  ${TELECODEC_SRC_DIR}/util/Timer.cpp
  ${TELECODEC_SRC_DIR}/util/Logger.cpp
)
target_include_directories(telehealth_util PUBLIC ${TELECODEC_INCLUDE_DIR})

# ========== Applications ==========
if(TELECODEC_BUILD_APPS)
  add_executable(encode_cli apps/encode_cli.cpp)
  target_link_libraries(encode_cli PRIVATE telehealth_codec telehealth_io telehealth_util)
  if(TELECODEC_USE_FFMPEG)
    target_link_libraries(encode_cli PRIVATE telehealth_io)
  endif()

  add_executable(live_stream_sender apps/live_stream_sender.cpp)
  target_link_libraries(live_stream_sender PRIVATE telehealth_codec telehealth_pipeline telehealth_io telehealth_util)

  add_executable(live_stream_receiver apps/live_stream_receiver.cpp)
  target_link_libraries(live_stream_receiver PRIVATE telehealth_codec telehealth_io telehealth_util)
endif()

# ========== Decode CLI (for roundtrip) ==========
if(TELECODEC_BUILD_APPS)
  add_executable(decode_cli apps/decode_cli.cpp)
  target_link_libraries(decode_cli PRIVATE telehealth_codec telehealth_io telehealth_util)
endif()

# ========== Tests ==========
if(TELECODEC_BUILD_TESTS)
  enable_testing()
  add_executable(test_yuv_conversion tests/test_yuv_conversion.cpp)
  target_link_libraries(test_yuv_conversion PRIVATE telehealth_codec telehealth_util)
  add_test(NAME test_yuv_conversion COMMAND test_yuv_conversion)

  add_executable(test_block_iterator tests/test_block_iterator.cpp)
  target_link_libraries(test_block_iterator PRIVATE telehealth_codec telehealth_util)
  add_test(NAME test_block_iterator COMMAND test_block_iterator)

  add_executable(test_motion_search tests/test_motion_search.cpp)
  target_link_libraries(test_motion_search PRIVATE telehealth_codec telehealth_util)
  add_test(NAME test_motion_search COMMAND test_motion_search)

  add_executable(test_bitstream_roundtrip tests/test_bitstream_roundtrip.cpp)
  target_link_libraries(test_bitstream_roundtrip PRIVATE telehealth_codec telehealth_io telehealth_util)
  add_test(NAME test_bitstream_roundtrip COMMAND test_bitstream_roundtrip)
endif()

# ========== Benchmarks ==========
if(TELECODEC_BUILD_BENCHMARKS)
  add_executable(bench_motion_search benchmarks/bench_motion_search.cpp)
  target_link_libraries(bench_motion_search PRIVATE telehealth_codec telehealth_util)

  add_executable(bench_end_to_end benchmarks/bench_end_to_end.cpp)
  target_link_libraries(bench_end_to_end PRIVATE telehealth_codec telehealth_io telehealth_util)
endif()
