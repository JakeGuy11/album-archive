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
#include <string>
#include <cstring>

std::string replaceString(std::string subject,const std::string& search,const std::string& replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

std::vector<std::string> splitIntoStrings(std::string inputString, std::string delimeter, int expectedOccurences)
{
	std::vector<std::string> returnVector;
	for (int i = 0; i < expectedOccurences; i++)
	{
		returnVector.push_back(inputString.substr(0, inputString.find(delimeter)));
		inputString.erase(0, inputString.find(delimeter) + delimeter.length());
	}
	return returnVector;
}

std::string getPrompt(std::string prompt)
{
	std::string returnString = "";
	std::cout << prompt << std::endl;
	std::cout << ">>>";
	//std::cin.ignore();
	std::getline(std::cin, returnString, '\n');
	return returnString;
}

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
	
	std::string firstCommandOutput = getCommandOutput(indexCountCommand.c_str());
	std::vector<std::string> firstCommandOutputVector = splitIntoStrings(firstCommandOutput, ";;", 4);
	int numberOfVideos = std::stoi(firstCommandOutputVector[0]);
	
	std::string pathToSave = "~/Music/" + getPrompt("Enter the path (relative to ~/Music/ where you would like to save the album");
	std::string albumYear = getPrompt("Enter the album year");
	std::string albumName = getPrompt("The current identified album name is " + firstCommandOutputVector[1] + ", would you like to change it? Leave blank for no, or enter its new name.");
	std::string albumArtist = getPrompt("The current identified artist is " + firstCommandOutputVector[2] + ", would you like to change it? Leave blank for no, or enter its new name.");
	if(albumName == "") albumName = firstCommandOutputVector[1];
	if(albumArtist == "") albumArtist = firstCommandOutputVector[2];
	
	//Download the image
	std::string coverCommand = "curl \"" + firstCommandOutputVector[3] + "\" --create-dirs -o " + replaceString(pathToSave, " ", "\\ ") + "/cover";
	system(coverCommand.c_str());
	
	for(int i = 0; i < numberOfVideos; i++)
	{
		std::cout << std::endl;
		std::string currentCommand = "./scripts/return_youtube_details.js \"" + playlistURL + "\" " + std::to_string(i);
		std::string commandOutput = getCommandOutput(currentCommand.c_str());
		std::vector<std::string> commandOutputVector = splitIntoStrings(commandOutput, ";;", 2);
		
		std::string chosenTitle = getPrompt("The current video title is " + commandOutputVector[1] + ", would you like to change it? Leave blank for no, or enter its new name.");
		if (chosenTitle == "") chosenTitle = commandOutputVector[1];
		
		std::string downloadCommand = "./scripts/extract_audio.py \"" + commandOutputVector[0] + "\" \"" + pathToSave + "\" \"" + chosenTitle + "\" \"" + albumName + "\" \"" + albumArtist + "\" \"" + albumYear + "\" " + std::to_string(i+1);
		system(downloadCommand.c_str());
		std::cout << "Finished downloading " << chosenTitle << std::endl;
	}

	std::string removeCoverCommand = "rm " + replaceString(pathToSave, " ", "\\ ") + "/cover";
	system(removeCoverCommand.c_str());
}
