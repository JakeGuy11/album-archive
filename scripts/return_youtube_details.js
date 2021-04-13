#!/usr/bin/node
// For the number of videos in the playlist (returned by parse_youtube_page.js), main.js will pass in the index of the video and the playlist url
// This script will return a whole bunch of details about the video at the index, including the title, artist, album name, number, etc.

const XMLHttpRequest = require("xmlhttprequest").XMLHttpRequest;

function getSourceAsDOM(url)
{
		xmlhttp = new XMLHttpRequest();
		xmlhttp.withCredentials = false;
		xmlhttp.open("GET", url, false);
		xmlhttp.send();
		return xmlhttp.responseText;
}

var rawPageDOM = getSourceAsDOM(process.argv[2]);
var desiredJSONPartOne = rawPageDOM.split("var ytInitialData = ")[1];
var desiredJSON = desiredJSONPartOne.split(";</script>")[0];

var entireJSON = JSON.parse(desiredJSON);
var neededEntries = entireJSON.contents.twoColumnBrowseResultsRenderer.tabs[0].tabRenderer.content.sectionListRenderer.contents[0].itemSectionRenderer.contents[0].playlistVideoListRenderer.contents;

var baseEntry = neededEntries[process.argv[3]].playlistVideoRenderer;

var videoID = baseEntry.videoId;
var videoTitle = baseEntry.title.runs[0].text;
var videoArtist = baseEntry.shortBylineText.runs[0].text;

console.log(videoID + ";;" + videoTitle);

