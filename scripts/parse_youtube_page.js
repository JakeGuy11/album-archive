#!/usr/bin/node
// This script will be called once at the start by main.cpp
// it will be given a youtube url, and it will return the number of videos in the playlist
// It will also download the thumbnail and return the absolute path to it, so main.js can later pass that path and add it to each audio file

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

//This is the giant JSON that has all the info we need
// console.log(desiredJSON);

var entireJSON = JSON.parse(desiredJSON);
var neededEntries = entireJSON.contents.twoColumnBrowseResultsRenderer.tabs[0].tabRenderer.content.sectionListRenderer.contents[0].itemSectionRenderer.contents[0].playlistVideoListRenderer.contents;

console.log(neededEntries.length);