// Just so that I don't forget, here is the inteded framework for this application
// 
// The user will run this application with the url of the youtube playlist in the command line (and eventually other options)
// This program will then pass that url to parse_youtube_page.js, which will return two things; the number of videos in that playlist and the absolute location to the thumbnail it downloaded.
// This program will then, for each video, send the playlist url and the index to return_youtube_details.js
// That will return a whole bunch of details, all which will be used in the tagging of the files
// From there, it will pass all of that information to extract_audio.py, which will use ffmpeg and it's metadata flag to download the audio and tag it.
#include <iostream>


int main()
{
	std::cout << "Starting Program..." << std::endl;
}
