cmake_minimum_required(VERSION 3.14)
project(CLion)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-lopus")

add_executable(CLion src/AudioGrabberALSA.cpp src/AudioGrabberALSA.h src/AudioSignalParams.h src/AudioFrame.h src/AudioReplayALSA.cpp src/AudioReplayALSA.h src/AudioTest.cpp src/IAudioFrameProducer.h src/AudioEncoderOpus.cpp src/AudioEncoderOpus.h src/EncodedAudio.h src/IAudioEncoded.h src/AudioDecoderOpus.cpp src/AudioDecoderOpus.h src/IAudioDecoded.h src/RTPClient.cpp src/RTPClient.h src/IClientCallback.h src/sha256.h src/sha256.cpp)
target_link_libraries(CLion /usr/local/lib/libjrtp.a)
target_link_libraries(CLion /usr/local/lib/libjthread.a)

find_package(ALSA REQUIRED)
if (ALSA_FOUND)
    include_directories(${ALSA_INCLUDE_DIRS})
    target_link_libraries (CLion ${ALSA_LIBRARIES})
endif(ALSA_FOUND)

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(CLion PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(CLion "${CMAKE_THREAD_LIBS_INIT}")
endif()

target_link_libraries(CLion opus)