///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017, STEREOLABS.
//
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

// ZED includes
#include <sl/Camera.hpp>

// Sample includes
#include <iostream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include "utils.hpp"

// Using namespace
using namespace sl;
using namespace std;

int main(int argc, char **argv) {
 
    if (argc != 3 ) {
        cout << "Usage: \n\n";
        cout << "    ZED_SVO_Export A B\n\n";
        cout << "Please use the following parameters from the command line:\n";
        cout << " A - SVO file path (input) : \"path/to/file.svo\"\n";
        cout << " B - AVI file path (output) or image sequence folder(output) : \"path/to/output/file.avi\" or \"path/to/output/folder\"\n";
        cout << " A and B need to end with '/' or '\\'\n\n";
        cout << "Examples: \n";
        cout << "  ZED_SVO_Export \"path/to/file.svo\" \"path/to/output/file.avi\" \n";
        cout << "\nPress [Enter] to continue";
        cin.ignore();
        return 1;
    }

    // Get input parameters
    string svo_input_path(argv[1]);
    string output_path(argv[2]);
    bool output_as_video = true;
    
    if (!output_as_video && !directoryExists(output_path)) {
        cout << "Input directory doesn't exist. Check permissions or create it.\n" << output_path << "\n";
        return 1;
    }

    if(!output_as_video && output_path.back() != '/' && output_path.back() != '\\') {
        cout << "Output folder needs to end with '/' or '\\'.\n" << output_path << "\n";
        return 1;
    }

    // Create ZED objects
    Camera zed;

    // Specify SVO path parameter
    InitParameters initParameters;
    initParameters.input.setFromSVOFile(svo_input_path.c_str());
    initParameters.coordinate_units = sl::UNIT::MILLIMETER;
    initParameters.depth_mode = sl::DEPTH_MODE::NONE;

    // Open the SVO file specified as a parameter
    ERROR_CODE err = zed.open(initParameters);
    if (err != ERROR_CODE::SUCCESS) {
        cout << toString(err) << endl;
        zed.close();
        return 1; // Quit if an error occurred
    }

    // Get image size
    Resolution image_size = zed.getCameraInformation().camera_resolution;
    int width = image_size.width;
    int height = image_size.height;
    cv::Size image_size_cv(width, height);
    sl::Mat left_image;
   
    // Create video writer
    cv::VideoWriter* video_writer;
    if (output_as_video) {
        int fourcc = cv::VideoWriter::fourcc('M', '4', 'S', '2'); // MPEG-4 part 2 codec
        int frame_rate = fmax(zed.getCameraInformation().camera_fps, 30); // Minimum write rate in OpenCV is 25
        video_writer = new cv::VideoWriter(output_path, fourcc, frame_rate, image_size_cv);
        if (!video_writer->isOpened()) {
            cout << "OpenCV video writer cannot be opened. Please check the .avi file path and write permissions." << endl;
            zed.close();
            return 1;
        }
    }

    // Start SVO conversion to AVI/SEQUENCE
    cout << "Converting SVO... Use Ctrl-C to interrupt conversion." << endl;

    int nb_frames = zed.getSVONumberOfFrames();
    int svo_position = 0;

    SetCtrlHandler();

    while (!exit_app) {
        if (zed.grab() == sl::ERROR_CODE::SUCCESS) {
            svo_position = zed.getSVOPosition();

            // Retrieve SVO images
            //zed.retrieveImage(left_image, VIEW::LEFT_UNRECTIFIED);
            zed.retrieveImage(left_image, VIEW::LEFT);
            cv::Mat left_image_ocv = slMat2cvMat(left_image);
    
            // Convert SVO image from RGBA to RGB
            cv::cvtColor(left_image_ocv, left_image_ocv, cv::COLOR_RGBA2RGB);

            // Write the RGB image in the video
            video_writer->write(left_image_ocv);
            
            // Display progress
            ProgressBar((float) (svo_position / (float) nb_frames), 30);

            // Check if we have reached the end of the video
            if (svo_position >= (nb_frames - 1)) { // End of SVO
                cout << "\nSVO end has been reached. Exiting now.\n";
                exit_app = true;
            }
        }
    }
    if (output_as_video) {
        // Close the video writer
        video_writer->release();
        delete video_writer;
    }

    zed.close();
    return 0;
}

