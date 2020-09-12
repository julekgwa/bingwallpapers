#!/bin/bash

sed -e 's/:[^:\/\/]/="/g;s/$/"/g;s/ *=/=/g' "`dirname $0`/config.yaml"> "`dirname $0`/config.sh"

source "`dirname $0`/config.sh"

XML_URL="http://www.bing.com/HPImageArchive.aspx?format=xml&idx=0&n=1&mkt=$REGION"

NO_INTERNET="1"
# check if user is online
wget -q --tries=10 --timeout=20 --spider http://google.com
if [ ! $? -eq 0 ];
then
    NO_INTERNET="0"
fi

file_exists() {
    local save_dir=$3

    local default_pic=${1##*/}
    local desired_pic=${2##*/}

    if [ ! -f "$save_dir$default_pic" ] && [ ! -f "$save_dir$desired_pic" ];
    then
        return 0
    else
        return 1
    fi
}

BING="www.bing.com"

if [ "$WALLPAPER_DIR" == "0" ]; then
    WALLPAPER_DIR=$(eval echo ~${USER})"/Pictures/Bingwallpapers/"
fi

mkdir -p $WALLPAPER_DIR;

WALLPAPER_DIR=${WALLPAPER_DIR%/}'/'

WALLPAPER_OPTIONS="zoom"

WALLPAPER_RESOLUTION="_1920x1200"

WALLPAPER_EXTENTION=".jpg"

WALLPAPER_URL=$BING$(echo $(curl -s $XML_URL) | grep -oP "<urlBase>(.*)</urlBase>" | cut -d ">" -f 2 | cut -d "<" -f 1)$WALLPAPER_RESOLUTION$WALLPAPER_EXTENTION

DEFAULT_WALLPAPER_URL=$BING$(echo $(curl -s $XML_URL) | grep -oP "<url>(.*)</url>" | cut -d ">" -f 2 | cut -d "<" -f 1)


file_exists $DEFAULT_WALLPAPER_URL $WALLPAPER_URL $WALLPAPER_DIR

if [ $? -eq 0 ];
then
    if [[ `wget --max-redirect=0 -S --spider $WALLPAPER_URL  2>&1 | grep 'HTTP/1.1 200 OK'` ]];
    then
        FILE_PATH=$WALLPAPER_DIR${WALLPAPER_URL##*OHR.}
        curl -s -o $FILE_PATH $WALLPAPER_URL
    elif [[ `wget -S --spider $DEFAULT_WALLPAPER_URL  2>&1 | grep 'HTTP/1.1 200 OK'` ]];
    then
        FILE_PATH=$WALLPAPER_DIR${DEFAULT_WALLPAPER_URL##*/}
        curl -s -o $FILE_PATH $DEFAULT_WALLPAPER_URL
    fi
fi


if [ -z "${FILE_PATH}" ]; then
    if [ "$ROTATE" -ne "0" ] || [ "$NO_INTERNET" == "0" ]; then
        FILE_PATH=$(ls "$WALLPAPER_DIR"*.jpg | shuf -n 1)
    fi
fi


if [ ! -z "${FILE_PATH-}" ]; then
    if [ $SET_BACKGROUND == "true" ];
    then
        # set background image
        DISPLAY=:0 GSETTINGS_BACKEND=dconf gsettings set org.gnome.desktop.background picture-uri '"file://'$FILE_PATH'"'
        DISPLAY=:0 GSETTINGS_BACKEND=dconf gsettings set org.gnome.desktop.background picture-options $WALLPAPER_OPTIONS
    fi
    if [ $SET_LOCKSCREEN == "true" ];
    then
        # set lock screen image
        DISPLAY=:0 GSETTINGS_BACKEND=dconf gsettings set org.gnome.desktop.screensaver picture-uri '"file://'$FILE_PATH'"'
        DISPLAY=:0 GSETTINGS_BACKEND=dconf gsettings set org.gnome.desktop.screensaver picture-options $WALLPAPER_OPTIONS
    fi
fi

if [ "$SET_DELETE_DAYS" -ne "0" ]; then
	# Remove pictures older than 7 days
    DEL="find ${WALLPAPER_DIR} -type f -mtime +${SET_DELETE_DAYS} -exec rm {} \;"
    eval $DEL
fi
echo ${FILE_PATH##*/}
exit # exit the script
