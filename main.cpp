// Just so that I don't forget, here is the inteded framework for this application
// 
// The user will run this application with the url of the youtube playlist in the command line (and eventually other options)
// This program will then pass that url to parse_youtube_page.js, which will return two things; the number of videos in that playlist and the absolute location to the thumbnail it downloaded.
// This program will then, for each video, send the playlist url and the index to return_youtube_details.js
// That will return a whole bunch of details, all which will be used in the tagging of the files
// From there, it will pass all of that information to extract_audio.py, which will use ffmpeg and it's metadata flag to download the audio and tag it.
#include <iostream>
#include <array>
#include <memory>
#include <algorithm>
#include <vector>

std::string getCommandOutput(const char* cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
	return result;
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		std::cerr << "Usage: " << std::endl
	          << argv[0] << " URL"
		  << std::endl;
		return 1;
	}

	std::string playlistURL = std::string(argv[1]);

	std::string indexCountCommand = "./scripts/parse_youtube_page.js \"" + playlistURL + "\"";
	
	int numberOfVideos = std::stoi(getCommandOutput(indexCountCommand.c_str()));

	for(int i = 0; i < numberOfVideos; i++)
	{
		std::string currentCommand = "./scripts/return_youtube_details.js \"" + playlistURL + "\" " + std::to_string(i);
		std::string commandOutput = getCommandOutput(currentCommand.c_str());
		std::string videoId = commandOutput.substr(0, commandOutput.find(";;"));
		commandOutput.erase(0, commandOutput.find(";;") + 2);
		std::string videoTitle = commandOutput.substr(0, commandOutput.find(";;"));
		commandOutput.erase(0, commandOutput.find(";;") + 2);
		std::string videoArtist = commandOutput.substr(0, commandOutput.find(";;"));

		std::cout << "\nVideo ID: " << videoId
			  << "\nVideo Title: " << videoTitle
			  << "\nVideo Artist:" << videoArtist
			  << std::endl;
	}
}
