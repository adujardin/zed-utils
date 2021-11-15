#include <sl/Camera.hpp>

using namespace sl;
static bool exit_app = false;

// Handle the CTRL-C keyboard signal
#ifdef _WIN32
#include <Windows.h>
void CtrlHandler(DWORD fdwCtrlType) {
    exit_app = (fdwCtrlType == CTRL_C_EVENT);
}
#else
#include <signal.h>
void nix_exit_handler(int s) {
    exit_app = true;
}
#endif

// Set the function to handle the CTRL-C
void SetCtrlHandler() {
#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE);
#else // unix
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = nix_exit_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
#endif
}

int main(int argc, char **argv) {

    // Create a ZED camera object
    Camera zed;
    
    // Set configuration parameters for the ZED
    InitParameters init_parameters;
    init_parameters.camera_resolution = sl::RESOLUTION::HD1080;
    init_parameters.depth_mode = DEPTH_MODE::NONE;
    init_parameters.sdk_verbose = true;

    // Open the camera
    ERROR_CODE returned_state = zed.open(init_parameters);
    if (returned_state != ERROR_CODE::SUCCESS) {
        std::cout << "Error " << returned_state << ", exit program.\n";
        return EXIT_FAILURE;
    }

    StreamingParameters stream_params;
    stream_params.codec = STREAMING_CODEC::H265;
    stream_params.bitrate = 8000;
    stream_params.chunk_size = 4096;

    returned_state = zed.enableStreaming(stream_params);
    if (returned_state != ERROR_CODE::SUCCESS) {
        std::cout << "Error " << returned_state << ", exit program.\n";
        return EXIT_FAILURE;
    }

    SetCtrlHandler();

    while(!exit_app) {
        if (zed.grab() != ERROR_CODE::SUCCESS)
            sleep_ms(1);
    }
    
    zed.disableStreaming();

    // Close the camera
    zed.close();
    return EXIT_SUCCESS;
}

