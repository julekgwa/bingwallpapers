# Bingwallpapers for Linux

![Bing Wallpaper](bing.png)

## Information

The script will update your wallpaper, when you login in to your pc, using [Bing's](https://bingwallpaper.com/) wallpaper of the day.

## How to use

```
$ git clone https://github.com/julekgwa/Bingwallpapers.git
$ cd Bingwallpapers
$ sudo sh install.sh
```
logout and login or run the script manually
``$ sh ~/Pictures/Bingwallpapers/bing.sh``

# Setting Up Cron

To setup hourly checks for new wallpapers, copy the script to ``/etc/cron.hourly/`` directory:

`` $ sudo cp /path/to/script/ /etc/cron.hourly``

[how to setup a cron job](https://askubuntu.com/questions/2368/how-do-i-set-up-a-cron-job).  You can use this [link](https://crontab-generator.org/) for reference.

# Contributing
Feel free to make changes and send PR, I'll be accepting them.

# License
MIT

