#define BOOST_LOG_DYN_LINK 1

#include <pix2image.h>
#include <boost/log/trivial.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>
//#include <photonfocus_camera.h>


namespace POLPro
{
    std::vector<cv::Mat> raw2mat(const cv::Mat& origin, const bool show=true) {
        // define the size of the output
        cv::Size output_size(origin.cols / 2, origin.rows / 2);
        // declare the vector containing the 4 angles images
        const int nb_angles = 4;
        std::vector<cv::Mat> output_img(nb_angles);
        for (auto it = output_img.begin(); it != output_img.end(); ++it)
            *it = cv::Mat::zeros(output_size, CV_16U);
        
        // copy the data in the new image
        for (int angle = 0; angle < nb_angles; ++angle) {
            int offset_row = angle / 2;
            int offset_col = angle % 2;

            for (int row = 0; row < origin.rows/2; ++row)
                for (int col = 0; col < origin.cols/2; ++col)
		  output_img[angle].at<unsigned short>(row, col) = origin.at<unsigned short>(2 * row + offset_row, 2 * col + offset_col);

            BOOST_LOG_TRIVIAL(debug) << minmax(output_img[angle], "I-");
            
        }

        if (show)
            imshow(output_img, false, false);
            

        return output_img;
    }

    std::vector<cv::Mat> compute_stokes(const std::vector<cv::Mat>& angles_img,
                                        const bool show=true) {
        // define the number of images to have for Stokes
        const int nb_stokes_img = 3;
        // Create zeros images
        std::vector<cv::Mat> output_img(nb_stokes_img);
        for (auto it = output_img.begin(); it != output_img.end(); ++it)
            *it = cv::Mat::zeros(angles_img[0].size(), CV_32F);

        // compute the Stokes parameters maps
        // S0: add the different angles
        for (auto it = angles_img.begin(); it != angles_img.end(); ++it)
            cv::add(output_img[0], *it, output_img[0], cv::noArray(),
                    CV_32F);
        output_img[0] /= 2.0;
        BOOST_LOG_TRIVIAL(debug) << minmax(output_img[0], "s0");

        // S1: subtract angles 0 and 90
        cv::subtract(angles_img[0], angles_img[3], output_img[1],
                     cv::noArray(), CV_32F);
        BOOST_LOG_TRIVIAL(debug) << minmax(output_img[1], "s1");

        // S2: subtract angles 45 and 135
        cv::subtract(angles_img[1], angles_img[2], output_img[2],
                     cv::noArray(), CV_32F);
        BOOST_LOG_TRIVIAL(debug) << minmax(output_img[2], "s2");

        if (show)
            imshow(output_img, false, true);

        return output_img;
    }

    std::vector<cv::Mat> compute_stokes(const cv::Mat& origin,
                                        const bool show=true) {
        // refactor the raw image
        std::vector<cv::Mat> angles_img = raw2mat(origin, show);

        return compute_stokes(angles_img, show);
    }

    std::vector<cv::Mat> compute_polar_params(
        const std::vector<cv::Mat>& origin, const bool show=true) {
        std::vector<cv::Mat> stokes_img;
        // Check if we have the original data or the stokes
        if (origin.size() == 4) {
            stokes_img = compute_stokes(origin, show);
        } else {
            stokes_img = origin;
        }

        // define the number of maps
        const int nb_params = 3;
        // create the zeros images
        std::vector<cv::Mat> output_img(nb_params);
        for (auto it = output_img.begin(); it != output_img.end(); ++it)
            *it = cv::Mat::zeros(stokes_img[0].size(), CV_32F);

        // compute the polar coordinate in degrees
        cv::cartToPolar(stokes_img[1], stokes_img[2],
                        output_img[0], output_img[1],
                        true);
        // normalize the maps
        // degree of polarization
        output_img[0] /= stokes_img[0];
        BOOST_LOG_TRIVIAL(debug) << minmax(output_img[0], "DoP");
        // angle of polarization
        output_img[1] *= 0.5;
        BOOST_LOG_TRIVIAL(debug) << minmax(output_img[1], "AoP");
        // copy s0
        stokes_img[0].copyTo(output_img[2]);
        BOOST_LOG_TRIVIAL(debug) << minmax(output_img[2], "s0");

        if (show)
            imshow(output_img, false, false);

        return output_img;
    }

    std::vector<cv::Mat> compute_polar_params(const cv::Mat& origin,
                                              const bool show=true) {
        // compute the Stokes' parameters
        std::vector<cv::Mat> stokes_img = compute_stokes(origin, show);

        return compute_polar_params(stokes_img, show);
    }

    std::string minmax(const cv::Mat& img, const std::string& s) {
        double min, max;
        cv::Point idmin, idmax;
        cv::minMaxLoc(img, &min, &max, &idmin, &idmax) ;

        return "Image " + s
            + ": min=" + std::to_string(min)
            + " - max= " + std::to_string(max);
    }


    std::vector<cv::Mat> polar_stokes_preprocessing(std::vector<cv::Mat> img, 
                                                   const bool is_stokes=true){

        // Convert the data if Stokes or polarization parameters
        if (is_stokes) {
            // Stokes parameters normalization
            img[0] /= 2.0;
            img[1] = (img[1] + (cv::pow(2, 16)-1)) / 2.0;
            img[2] = (img[2] + (cv::pow(2, 16)-1)) / 2.0;

            BOOST_LOG_TRIVIAL(debug) << "The min and maximum of the converted image : \n "  ;
            for (int i = 0; i < img.size(); ++i){
                img[i].convertTo(img[i], CV_16U);
                BOOST_LOG_TRIVIAL(debug) << minmax(img[i], "Img" + std::to_string(i));
            }

        } else {
            // polarization parameters normalization
            img[0] = img[0] * (cv::pow(2, 16) -1);
            //angle is between 0 and 180 to show it in the CV_16U we just applify it by the maxVal/2
            img[1] = img[1]; //+ (cv::pow(2, 10) -1);
            img[2] = img[2] / 2;
            img[0].convertTo(img[0], CV_16U); 
            img[1].convertTo(img[1], CV_8U); 
            img[2].convertTo(img[2], CV_16U); 

            
        }
        // Convert to uint16
        
            
   
        return img; 
    }

    void imshow(std::vector<cv::Mat> img, const bool as_hsv=false,
                const bool is_stokes=true) {

        // through an error if there is not 3d img and hsv is turned on
        if ((img.size() != 3) && as_hsv)
            throw std::invalid_argument("img needs to be a 3 channels images"
                                        " if you need hsv support");
       
        // Preprocessing img 
        if (img.size() ==3){
             // Convert the data if Stokes or polarization parameters
            if (is_stokes) {
                // Stokes parameters normalization
                img[0] /= 2.0;
                img[1] = (img[1] + (cv::pow(2, 16)-1)) / 2.0;
                img[2] = (img[2] + (cv::pow(2, 16)-1)) / 2.0;
            } else {
                // polarization parameters normalization
                img[0] = img[0] * (cv::pow(2, 16) -1);
                //angle is between 0 and 180 to show it in the CV_16U we just applify it by the maxVal/2
                img[1] = img[1]; // +(cv::pow(2, 10) -1);
                img[2] = img[2] / 2;
            }
        // Convert to uint16
        
            for (int i = 0; i < img.size(); ++i){
                img[i].convertTo(img[i], CV_16U);
            }
   
        }

        // Declare the output image
        cv::Mat output_img;

        if (as_hsv) {
            // Merge the image together to have a 3 channels image

            std::vector<cv::Mat> channels;
            channels.push_back(img[1].clone()); 
            channels.push_back(img[0].clone()); 
            channels.push_back(img[2].clone());

            cv::Mat bgr_img;
            cv::merge(channels, bgr_img);
            cv::cvtColor(bgr_img, output_img, CV_HLS2BGR);
        } else {
            // Concatenate the images available together
            cv::Size img_size(img[0].cols, img[0].rows);
            output_img = cv::Mat::zeros
                (img[0].rows*2, img[0].cols*2, CV_16U);
            int rows = img[0].rows; 
            int cols = img[0].cols;
            
    
            for (int i = 0; i < img.size(); ++i) {
                // we need to shift the image next to each other properly
                int offset_col = i % 2;
                int offset_row = i / 2;
               
                img[i].copyTo(output_img(
                                  cv::Rect(img_size.width * offset_col,
                                           img_size.height * offset_row,
                                           img_size.width,
                                           img_size.height)));

            }
        }

        cv::imshow("Output image", output_img);
    }

    void imsave(std::vector<cv::Mat> img, const std::string& s,
                const std::string method,
                const std::string PathtoSave){
        
        if(method == "stokes"){
            for (int i = 0; i < img.size(); ++i){
                cv::imwrite(PathtoSave+"S"+ std::to_string(i) + "_" + s, img[i]);
            }

        }else if(method == "polar"){
            for (int i = 0; i < img.size()-1; ++i){
                if (i == 0){
                    cv::imwrite(PathtoSave+ "DoP" + "_" + s, img[i]);
                }else{
                    cv::imwrite(PathtoSave+ "AoP" + "_" + s, img[i]);
                }
            }
        }else{
            int list[] = {0, 45, 135, 90}; 
            for (int i = 0; i < img.size(); ++i){
                
                cv::imwrite(PathtoSave+"I"+ std::to_string(list[i]) + "_" + s, img[i]);
            }            
 
        }
        
    }

}  // Namespace POLPro



using namespace boost::filesystem;       

int main(int argc, char  *argv[]) {   

    if (argc < 2) {
        std::cout <<" Usage: pix2image method['angle', 'stokes', 'polar'] PathtoLoad PathtoSave" << std::endl;
        return -1;
    }


    std::string method = argv[1]; 
    std::string LoadDir = argv[2];
    std::string SaveDir = argv[3]; 
    std::cout << "method --> " << method << std::endl; 
    std::cout << "LoadDir--> " << LoadDir << std::endl; 
    std::cout << "SaveDir --> " << SaveDir << std::endl; 
        
    // Loadin the images from a directory 
    std::vector<cv::String> fn;
    cv::glob(LoadDir, fn, false); 
    
    std::vector<cv::Mat> images;
    std::size_t count = fn.size(); //number of png files in images folder
    
    int count2 = 0; 
    
    for (size_t i=0; i<count; i++){
        std::cout<< "" << fn[i] << std::endl;
        //images.push_back(cv::imread(fn[i]));
        cv::Mat image = cv::imread(fn[i], CV_LOAD_IMAGE_UNCHANGED);
        // parsed image from original std
        std::vector<cv::Mat> output_img;
        if (method == "stokes"){
           // Stokes parameters
            output_img = POLPro::compute_stokes(image, false);
            output_img = POLPro::polar_stokes_preprocessing(output_img); 

        }else if (method == "polar"){
            output_img = POLPro::compute_polar_params(image, false);
            output_img = POLPro::polar_stokes_preprocessing(output_img, false); 
         
        }else if (method == "angle"){
            // angle image 
            output_img = POLPro::raw2mat(image, false);
            
        }else{
            std::cout<< "The method is not reconizable" << std::endl; 
            return -1; 
        }

        std::string name; 
        name = std::to_string(i) + ".tiff"; 
        POLPro::imsave(output_img, name, method, SaveDir);
        POLPro::imshow(output_img, false, true);
        cv::waitKey(0);
        count2 +=1; 
    }
    
    std::cout<< "Number of fils:" << count2 << std::endl; 
    

    return 0;                                    
}





