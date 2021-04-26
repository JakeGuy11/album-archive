#!/usr/bin/python
# This file will be passed in a whole bunch of details, one by one, about each video in the playlist. It will download the video and tag it with all the passed metadata
# Upon completion, it will return either 'success' or 'failure', possibly also types of failures
import sys
import os
import youtube_dl

requested_title = sys.argv[3] 
requested_path = sys.argv[2]
requested_album = sys.argv[4]
requested_artist = sys.argv[5]
requested_year = sys.argv[6]
requested_tracknum = sys.argv[7]

requested_url = sys.argv[1]

dl_opts = {
    'quiet': True,
    'format': 'bestaudio/best',
    'outtmpl': requested_path + "/currentitem"
}

with youtube_dl.YoutubeDL(dl_opts) as ydl:
    ydl.download([requested_url])

metadata_commands = "-metadata title=\"" + requested_title + "\" -metadata album=\"" + requested_album + "\" -metadata artist=\"" + requested_artist + "\" -metadata date=\"" + requested_year + "\" -metadata track=\"" + requested_tracknum + "\""

save_name = requested_path.replace(" ", "\ ").replace("(", "\(").replace(")", "\)").replace("'", "\\'").replace("\"", "\\\"") + "/" + requested_title.replace(" ", "\ ").replace("(", "\(").replace(")", "\)").replace("'", "\\'").replace("\"", "\\\"") + ".mp3"

ffmpeg_command = "ffmpeg -i \"" + os.path.expanduser(requested_path) + "/currentitem\" -i \"" + os.path.expanduser(requested_path) + "/cover\" -map 0:0 -map 1:0 -q:a 0 -y -id3v2_version 3 " + metadata_commands + " -metadata:s:v comment=\"Cover (front)\" " + os.path.expanduser(save_name)

os.system(ffmpeg_command)
rm_command = "rm " + requested_path.replace(" ", "\ ").replace("(", "\(").replace(")", "\)").replace("'", "\\'").replace("\"", "\\\"") + "/currentitem"
os.system(rm_command)

