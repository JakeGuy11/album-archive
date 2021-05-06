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
#include<filesystem>
#include<fstream>
// Here are some things we'll need globally
bool customCover = false;
std::string customCoverLocation = "";
// Here is the intent: by default, it's to download the album. The user can pass flags that will change it
std::string intent = "album";
// This is in case the user wants to include the disk number
std::string diskNum = "1";
// Run the program with debug?
bool verbose = false;

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
	          << argv[0] << "[flags] URL"
		  << std::endl;
		return 1;
	}

	for(int i = 1; i < argc; ++i)
	{
		if (std::string(argv[i]) == "-c" || std::string(argv[i]) == "--cover")
		{
			customCover = true;
			customCoverLocation = std::string(argv[i+1]);
		}
		else if (std::string (argv[i]) == "-s" || std::string (argv[i]) == "--single")
		{
			intent = "single";
			std::cout << "Mode set to single." << std::endl;
		}
		else if (std::string (argv[i]) == "-d" || std::string (argv[i]) == "--disk")
		{
			diskNum = std::string (argv[i+1]);
		}
		else if (std::string (argv[i]) == "-f" || std::string (argv[i]) == "--dir")
		{
			intent = "dir";
			std::cout << "Mode set to directory." << std::endl;
		}
		else if (std::string (argv[i]) == "-v" || std::string (argv[i]) == "--verbose")
		{
			verbose = true;
		}
	}
	

	if (intent == "album")
	{
		std::string playlistURL = std::string(argv[argc-1]);

		std::string indexCountCommand = "/opt/albumarchive/scripts/parse_youtube_playlist.js \"" + playlistURL + "\"";
		
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
		std::string coverCommand = "";
		if(customCover) coverCommand = "mkdir -p " + replaceString(pathToSave, " ", "\\ ") + " 2> /dev/null; cp " + replaceString(customCoverLocation, " ", "\\ ") + " " + replaceString(pathToSave, " ", "\\ ") + "/cover";
		else coverCommand = "curl \"" + firstCommandOutputVector[3] + "\" --create-dirs -o " + replaceString(pathToSave, " ", "\\ ") + "/cover 2> /dev/null";

		system(coverCommand.c_str());
		
		for(int i = 0; i < numberOfVideos; i++)
		{
			std::cout << std::endl;
			std::string currentCommand = "/opt/albumarchive/scripts/return_playlist_details.js \"" + playlistURL + "\" " + std::to_string(i);
			std::string commandOutput = getCommandOutput(currentCommand.c_str());
			std::vector<std::string> commandOutputVector = splitIntoStrings(commandOutput, ";;", 2);
			
			std::string chosenTitle = getPrompt("The current video title is " + commandOutputVector[1] + ", would you like to change it? Leave blank for no, or enter its new name.");
			if (chosenTitle == "") chosenTitle = commandOutputVector[1];
			
			std::string downloadCommand = "/opt/albumarchive/scripts/extract_audio.py \"https://www.youtube.com/watch?v=" + commandOutputVector[0] + "\" \"" + pathToSave + "\" \"" + chosenTitle + "\" \"" + albumName + "\" \"" + albumArtist + "\" \"" + albumYear + "\" " + std::to_string(i+1);
			system(downloadCommand.c_str());
			std::cout << "Finished downloading " << chosenTitle << std::endl;
		}

		std::string removeCoverCommand = "rm " + replaceString(pathToSave, " ", "\\ ") + "/cover";	
		system(removeCoverCommand.c_str());
		
		return 0;
	}
	else if (intent == "single")
	{
		std::string videoURL = std::string(argv[argc-1]);

		// Get all the details of the single here
		std::string singleTitle = getPrompt("Enter the title of the song");
		std::string singlePath = "~/Music/" + getPrompt("Enter the path, relative to ~/Music, you would like to save the single");
		std::string singleName = getPrompt("Enter the name of the single if it is not the same as the song title. If it is, leave this field blank");
		if (singleName == "") singleName = singleTitle;
		std::string singleArtist = getPrompt("Enter the artist of the single");
		std::string singleYear = getPrompt("Enter the year of the single");
		std::string singleIndex = getPrompt("Enter the index of this song in the single if it is not the only song, otherwise leave it blank");
		if (singleIndex == "") singleIndex = "1";

		// Make the directory if it does not exist
		std::string mkdirCommand = "mkdir -p " + replaceString (singlePath, " ", "\\ ") + " 2> /dev/null";
		system(mkdirCommand.c_str());

		// Download the cover here
		std::string coverURL;
		std::string coverCommand;
		if (!customCover) coverURL = getPrompt ("Enter the url of the cover image");
		if (customCover) coverCommand = "cp " + replaceString(customCoverLocation, " ", "\\ ") + " " + replaceString(singlePath, " ", "\\ ") + "/cover";
		else coverCommand = "curl \"" + coverURL + "\" --create-dirs -o " + replaceString(singlePath, " ", "\\ ") + "/cover 2> /dev/null";
		system(coverCommand.c_str());
	
		//Generate and execute the extraction command
		std::string extractArguments = "\"" + videoURL + "\" \"" + singlePath + "\" \"" + singleTitle + "\" \"" + singleName + "\" \"" + singleArtist + "\" \"" + singleYear + "\" \"" + singleIndex + "\" \"" + diskNum + "\"";
		std::string extractCommand = "";
		if (verbose)  extractCommand = "/opt/albumarchive/scripts/extract_audio.py " + extractArguments;
		else extractCommand = "/opt/albumarchive/scripts/extract_audio.py " + extractArguments + " 2> /dev/null";
		if (verbose) std::cout << "Extraction command: " << extractCommand << std::endl;
		system(extractCommand.c_str());
		
		//Remove the cover
		std::string removeCoverCommand = "rm " + replaceString (singlePath, " ", "\\ ") + "/cover";
		system(removeCoverCommand.c_str());

		return 0;
	}
	else if (intent == "dir")
	{
		std::string dirPath = std::string (argv[argc-1]);

		// Get all the user arguments
		std::string albumName = getPrompt ("Enter the name of the album");
		std::string savePath = "~/Music/" + getPrompt ("Enter the path, relative to ~/Music/, you would like to save the album");
		std::string albumArtist = getPrompt ("Enter the name of the artist");
		std::string albumYear = getPrompt ("Enter the year of the album");
		std::string naturalIndexString = getPrompt ("Are the songs in order naturally? (y/N)");
		while (naturalIndexString != "y" && naturalIndexString != "n" && naturalIndexString != "Y" && naturalIndexString != "N") naturalIndexString = getPrompt ("Input not recognized. Are the songs in order naturally? (y/N)");
		bool naturalIndex = false;
		if (naturalIndexString == "y" || naturalIndexString == "Y") naturalIndex = true;

		// Make the folder if it does not exist
		std::string mkdirCommand = "mkdir -p " + replaceString (savePath, " ", "\\ ");
		system (mkdirCommand.c_str());

		// Download the cover
		std::string coverURL;
		std::string coverCommand;
		if (!customCover) coverURL = getPrompt ("Enter the url of the cover image");
		if (customCover) coverCommand = "cp " + replaceString (customCoverLocation, " ", "\\ ") + " " + replaceString (savePath, " ", "\\ ") + "/cover";
		else coverCommand = "curl \"" + coverURL + "\" --create-dirs -o " + replaceString (savePath, " ", "\\ ") + "/cover 2> /dev/null";
		system (coverCommand.c_str());

		int songIndex = 1;
		for (const auto & file : std::filesystem::directory_iterator(dirPath))
		{
			std::cout << "\nOn song " << file.path()  << std::endl;
			
			// Get user input
			std::string songTitle = getPrompt ("Enter the title of the song (or enter \"_PASS\" to skip this file)");
			if (songTitle == "_PASS") continue;
			std::string currentIndex = std::to_string(songIndex);
			if (!naturalIndex) currentIndex = getPrompt ("Enter the index of the current song");
			
			// Generate the extraction command
			std::string extractArguments = "\"" + std::string(file.path()) + "\" \"" + savePath + "\" \"" + songTitle + "\" \"" + albumName + "\" \"" + albumArtist + "\" \"" + albumYear + "\" \"" + currentIndex + "\" \"" + diskNum + "\"";
			std::string extractCommand = "/opt/albumarchive/scripts/extract_audio.py " + extractArguments;
			if (verbose) std::cout << extractCommand << std::endl;
			else extractCommand += " 2> /dev/null";
			system (extractCommand.c_str());

			songIndex++;
		}

		// Remove the cover
		std::string removeCoverCommand = "rm " + replaceString (savePath, " ", "\\ ") + "/cover";
		system (removeCoverCommand.c_str());
	}
}








