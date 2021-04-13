#!/usr/bin/python
# This file will be passed in a whole bunch of details, one by one, about each video in the playlist. It will download the video and tag it with all the passed metadata
# Upon completion, it will return either 'success' or 'failure', possibly also types of failures
import sys
import os
import youtube_dl

try:
    requested_title = sys.argv[3] 
    requested_path = sys.argv[2]
    requested_album = sys.argv[4]
    requested_artist = sys.argv[5]
    requested_year = sys.argv[6]

    dl_opts = {
        'quiet': True,
        'format': 'bestaudio',
        'outtmpl': requested_path + "currentitem"
    }

    with youtube_dl.YoutubeDL(dl_opts) as ydl:
        ydl.download([sys.argv[1]])

    metadata_commands = "-metadata title=\"title test\""

    ffmpeg_command = "ffmpeg -i " + requested_path + "currentitem -y -loglevel quiet -crf 0 " + metadata_commands + " " + requested_path + requested_title + ".wav"

    os.system(ffmpeg_command)
    os.system("rm " + requested_path + "currentitem")
    print(0)
except:
    print(1)
