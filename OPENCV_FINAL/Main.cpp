// Main.cpp

#include "Main.h"
#include "conio.h"
using namespace std;
///////////////////////////////////////////////////////////////////////////////////////////////////
string folder;
int nextPlate = 0;

vector<string> menu = {
	"\n\n\n\n",
	"[1] Traverse through images and their recognition result in images one by one",
	"[2] Go through all the images to get % accuracy result in the dataset",
	"[Other key] Exit"
};
int main(void) {

	bool blnKNNTrainingSuccessful = loadKNNDataAndTrainKNN();           // attempt KNN training

	if (blnKNNTrainingSuccessful == false) {                            // if KNN training was not successful
																		// show error message
		std::cout << std::endl << std::endl << "error: error: KNN traning was not successful" << std::endl << std::endl;
		return(0);                                                      // and exit program
	}

	std::cout << "Path to dataset: ";
	getline(std::cin, folder);
	if (!boost::filesystem::exists(boost::filesystem::path(folder)))
	{
		std::cout << "The directory does not exist." << endl;
		return -1;
	}

	std::vector<std::string> names = get_file_list(folder);
	char choice = 0;
	char prevChoice;
	prevChoice = 0;

	create_folders({ "Success_images", "Partial_success_images", "Failed_images" });

	do {
		if (prevChoice != '1') {
			for (int iMenu = 0; iMenu < menu.size(); ++iMenu) {
				std::cout << menu[iMenu] << endl;
			}
			std::cout << "Your choice: ";
			prevChoice = choice;
			choice = _getch();
		}
		if (choice == '1')
		{
			cv::Mat imgOriginalScene;           // input image
			imgOriginalScene = cv::imread(folder + "\\" + names[nextPlate]);         // open image


			if (imgOriginalScene.empty()) {                             // if unable to open image
				std::cout << "error: image not read from file\n\n";     // show error message on command line
				continue;
			}

			std::vector<PossiblePlate> vectorOfPossiblePlates = detectPlatesInScene(imgOriginalScene);          // detect plates

			vectorOfPossiblePlates = detectCharsInPlates(vectorOfPossiblePlates);                               // detect chars in plates

																												//cv::imshow("imgOriginalScene", imgOriginalScene);           // show scene image

			if (vectorOfPossiblePlates.empty()) {                                               // if no plates were found
				std::cout << std::endl << "no license plates were detected" << std::endl;       // inform user no plates were found
			}
			else {
				// else
				string new_name = refineString(removeExtension(names[nextPlate]));
				// if we get in here vector of possible plates has at leat one plate

				// sort the vector of possible plates in DESCENDING order (most number of chars to least number of chars)
				std::sort(vectorOfPossiblePlates.begin(), vectorOfPossiblePlates.end(), PossiblePlate::sortDescendingByNumberOfChars);

				// suppose the plate with the most recognized chars (the first plate in sorted by string length descending order) is the actual plate
				PossiblePlate licPlate = vectorOfPossiblePlates.front();
				cv::imshow("imgPlate", licPlate.imgPlate);            // show crop of plate and threshold of plate
				cv::imshow("imgThresh", licPlate.imgThresh);

				if (licPlate.strChars.length() == 0) {                                                      // if no chars were found in the plate
					std::cout << std::endl << "no characters were detected" << std::endl << std::endl;      // show message
																											//return(0);                                 
				}
				else {
					std::cout << std::endl << "license plate read from image = " << licPlate.strChars << std::endl;     // write license plate text to std out
					std::cout << std::endl << "-----------------------------------------" << std::endl;
					drawRedRectangleAroundPlate(imgOriginalScene, licPlate);
					writeLicensePlateCharsOnImage(imgOriginalScene, licPlate);
					cv::imshow("imgResult", imgOriginalScene);
				}
			}

			for (int iMenu = 0; iMenu < menu.size(); ++iMenu) {
				std::cout << menu[iMenu] << endl;
			}
			std::cout << "Your choice: ";
			prevChoice = choice;
			choice = cv::waitKey(0);                 // hold windows open until user presses a key

			++nextPlate;
			nextPlate %= names.size();
		}
		else if (choice == '2')
		{

			std::ofstream fout, fail_out, success_out, partial_success_out;
			fout.open("result.txt");
			fail_out.open("failed.txt");
			success_out.open("success.txt");
			partial_success_out.open("partial_success.txt");
			int nSuccess = 0;
			int nFail = 0;
			int nPartialSuccess = 0;
			for (int i = 0; i < names.size(); ++i) {

				cv::Mat imgOriginalScene;           // input image
				imgOriginalScene = cv::imread(folder + "\\" + names[i]);         // open image

				if (imgOriginalScene.empty()) {                             // if unable to open image
					std::cout << "error: image not read from file\n\n";     // show error message on command line
					continue;
				}

				std::vector<PossiblePlate> vectorOfPossiblePlates = detectPlatesInScene(imgOriginalScene);          // detect plates

				vectorOfPossiblePlates = detectCharsInPlates(vectorOfPossiblePlates);                               // detect chars in plates

																													//cv::imshow("imgOriginalScene", imgOriginalScene);           // show scene image

				if (vectorOfPossiblePlates.empty()) {         // inform user no plates were found
					++nFail;
					fout << "License plate not found in " << names[i] << endl;
					fail_out << "License plate not found in " << names[i] << endl;
				}
				else {
					// else
					string new_name = refineString(removeExtension(names[i]));
					// if we get in here vector of possible plates has at leat one plate

					// sort the vector of possible plates in DESCENDING order (most number of chars to least number of chars)
					std::sort(vectorOfPossiblePlates.begin(), vectorOfPossiblePlates.end(), PossiblePlate::sortDescendingByNumberOfChars);

					// suppose the plate with the most recognized chars (the first plate in sorted by string length descending order) is the actual plate
					PossiblePlate licPlate = vectorOfPossiblePlates.front();
					vector<PossiblePlate> fairest;
					fairest.push_back(licPlate);
					for (int j = 1; j < vectorOfPossiblePlates.size(); ++j) {
						if (vectorOfPossiblePlates[j].strChars.size() == licPlate.strChars.size())
							fairest.push_back(vectorOfPossiblePlates[j]);
						else
							break;
					}
					//cv::imshow("imgPlate", licPlate.imgPlate);            // show crop of plate and threshold of plate
					//cv::imshow("imgThresh", licPlate.imgThresh);

					if (licPlate.strChars.length() == 0) {                                                      // if no chars were found in the plate
						std::cout << std::endl << "no characters were detected" << std::endl << std::endl;      // show message
																												//return(0);                                                                              // and exit program
						fout << new_name << ": cant detect" << endl;
						fail_out << new_name << ": cant detect" << endl;
						++nFail;
					}
					else {
						fout << '[';
						for (int j = 0; j < fairest.size(); ++j) {
							fout << " " << fairest[j].strChars << ", ";
						}
						fout << "] ";
						int compare_result = compareString(toLower(new_name), toLower(licPlate.strChars));
						drawRedRectangleAroundPlate(imgOriginalScene, licPlate);
						writeLicensePlateCharsOnImage(imgOriginalScene, licPlate);
						switch (compare_result) {
						case -1:
							fout << new_name << ": fell to " << licPlate.strChars << endl;
							fail_out << new_name << ": fell to " << licPlate.strChars << endl;
							++nFail;
							cv::imwrite("Failed_images/" + names[i], imgOriginalScene);

							break;
						case 0:
							fout << new_name << ": success" << endl;
							success_out << new_name << endl;
							++nSuccess;
							cv::imwrite("Success_images/" + names[i], imgOriginalScene);
							break;
						default:
							fout << new_name << ": partial success (" << compare_result << ")" << endl;
							partial_success_out << new_name << ": fell to " << licPlate.strChars << " (" << compare_result << ")" << endl;
							++nPartialSuccess;
							cv::imwrite("Partial_success_images/" + names[i], imgOriginalScene);
						}

						//drawRedRectangleAroundPlate(imgOriginalScene, licPlate);                // draw red rectangle around plate

						//std::cout << std::endl << "license plate read from image = " << licPlate.strChars << std::endl;     // write license plate text to std out
						//std::cout << std::endl << "-----------------------------------------" << std::endl;

						//writeLicensePlateCharsOnImage(imgOriginalScene, licPlate);              // write license plate text on the image

						//cv::imshow("imgOriginalScene", imgOriginalScene);                       // re-show scene image

						//cv::imwrite("Outs/" + new_name + ".jpg", imgOriginalScene);                  // write image out to file
					}
				}
				std::cout << i << "/" << names.size() << " completed" << endl;
				//cv::waitKey(0);                 // hold windows open until user presses a key
			}
			std::cout << endl << endl;
			fout << endl << endl;
			std::cout << "Number of Successes: " << nSuccess << endl;
			fout << "Number of Successes: " << nSuccess << endl;
			std::cout << "Number of Partial Successes: " << nPartialSuccess << endl;
			fout << "Number of Partial Successes: " << nPartialSuccess << endl;
			std::cout << "Number of Failures: " << nFail << endl;
			fout << "Number of Failures: " << nFail << endl;
			std::cout << " % Success: " << (float)(nSuccess + nPartialSuccess) / ((float)(nSuccess + nPartialSuccess + nFail)) * 100 << "%" << endl;
			fout << " % Success: " << (float)(nSuccess + nPartialSuccess) / ((float)(nSuccess + nPartialSuccess + nFail)) * 100 << "%" << endl;
			fout.close();
			fail_out.close();
			success_out.close();
			partial_success_out.close();
			std::cout << "Check out [result.txt], [success.txt], [failed,txt], [partial_success.txt] for further detail" << endl;
			prevChoice = choice;
		}
		else
			break;
	} while (true);
	return(0);
}

std::string toLower(string s) {
	std::locale loc;
	string ret = "";
	for (int i = 0; i < s.size(); ++i) {
		ret += tolower(s[i], loc);
	}
	return ret;
}

string removeExtension(string s) {
	size_t lastindex = s.find_last_of(".");
	return s.substr(0, lastindex);
}

string refineString(string s) {
	string result = "";
	for (int i = 0; i < s.size(); ++i) {
		if (s[i] != ' ' && s[i] != '_') {
			result += s[i];
		}
	}
	return result;
}

int compareString(string s, string t) {
	if (s.size() != t.size())
		return -1;
	int result = 0;
	for (int i = 0; i < s.size(); ++i) {
		if (s[i] == t[i])
			continue;
		if (s[i] == 'i' && t[i] == '1' || s[i] == '1' && t[i] == 'i')
			result += 1;
		else if (s[i] == '0' && t[i] == 'o' || s[i] == 'o' && t[i] == '0')
			result += 1;
		else if (s[i] == 's' && t[i] == '5' || s[i] == '5' && t[i] == 's')
			result += 1;
		else
			return -1;
	}
	return result;
}


std::vector<std::string> get_file_list(const std::string& path)
{
	std::vector<std::string> m_file_list;
	if (!path.empty())
	{
		namespace fs = boost::filesystem;

		fs::path apk_path(path);
		fs::recursive_directory_iterator end;

		for (fs::recursive_directory_iterator i(apk_path); i != end; ++i)
		{
			const fs::path cp = (*i);

			m_file_list.push_back(cp.filename().string());
		}
	}
	return m_file_list;
}


void create_folders(const std::vector<string>& folders)
{
	namespace fs = boost::filesystem;
	for (int i = 0; i < folders.size(); ++i)
	{
		fs::path path(folders[i]);
		if (!fs::exists(path))
			fs::create_directory(path);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawRedRectangleAroundPlate(cv::Mat &imgOriginalScene, PossiblePlate &licPlate) {
	cv::Point2f p2fRectPoints[4];

	licPlate.rrLocationOfPlateInScene.points(p2fRectPoints);            // get 4 vertices of rotated rect

	for (int i = 0; i < 4; i++) {                                       // draw 4 red lines
		cv::line(imgOriginalScene, p2fRectPoints[i], p2fRectPoints[(i + 1) % 4], SCALAR_RED, 2);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void writeLicensePlateCharsOnImage(cv::Mat &imgOriginalScene, PossiblePlate &licPlate) {
	cv::Point ptCenterOfTextArea;                   // this will be the center of the area the text will be written to
	cv::Point ptLowerLeftTextOrigin;                // this will be the bottom left of the area that the text will be written to

	int intFontFace = CV_FONT_HERSHEY_SIMPLEX;                              // choose a plain jane font
	double dblFontScale = (double)licPlate.imgPlate.rows / 30.0;            // base font scale on height of plate area
	int intFontThickness = (int)std::round(dblFontScale * 1.5);             // base font thickness on font scale
	int intBaseline = 0;

	cv::Size textSize = cv::getTextSize(licPlate.strChars, intFontFace, dblFontScale, intFontThickness, &intBaseline);      // call getTextSize

	ptCenterOfTextArea.x = (int)licPlate.rrLocationOfPlateInScene.center.x;         // the horizontal location of the text area is the same as the plate

	if (licPlate.rrLocationOfPlateInScene.center.y < (imgOriginalScene.rows * 0.75)) {      // if the license plate is in the upper 3/4 of the image
																							// write the chars in below the plate
		ptCenterOfTextArea.y = (int)std::round(licPlate.rrLocationOfPlateInScene.center.y) + (int)std::round((double)licPlate.imgPlate.rows * 1.6);
	}
	else {                                                                                // else if the license plate is in the lower 1/4 of the image
																						 // write the chars in above the plate
		ptCenterOfTextArea.y = (int)std::round(licPlate.rrLocationOfPlateInScene.center.y) - (int)std::round((double)licPlate.imgPlate.rows * 1.6);
	}

	ptLowerLeftTextOrigin.x = (int)(ptCenterOfTextArea.x - (textSize.width / 2));           // calculate the lower left origin of the text area
	ptLowerLeftTextOrigin.y = (int)(ptCenterOfTextArea.y + (textSize.height / 2));          // based on the text area center, width, and height

			 // write the text on the image
	cv::putText(imgOriginalScene, licPlate.strChars, ptLowerLeftTextOrigin, intFontFace, dblFontScale, SCALAR_YELLOW, intFontThickness);
}



