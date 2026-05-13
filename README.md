# AnimeMUD Modernization Project by Daniel W. Sutton

Basically, fixing old version of the original AnimeMud to run on modern C compilers.

AnimeMud is derived from Rom 2.4, Merc 2.1 and DikuMud

-----

## Local Dev
### Build and Run Locally on Ubunutu (or Ubuntu Desktop on PC)
cd /mnt/c/Users/<user>/AnimeMud/animemud/src (or whever your code is saved)
unzip animemud.zip
cd animemud/src
make CFLAGS="-g -Wall"
nohup ./rom > ../../rom.log 2>&1 &

-----

## In Game Commands
wizhelp
goto 3001 (Recall)
goto 3170 (Temple of Quests)
goto 26000 (Immortal Hangout)

-----

## Server Dev

This is assuming you have it deployed to AWS EC2 and have a copy of the .pem that gives you access to ssh or scp to the public port hosted on AWS.

### To Copy Code From PC To Server
cd to wherever you have the code and .pem saved
scp -i .\AnimeMud-KeyPair.pem .\animemud.zip ubuntu@<ip_address>:~

### To Build Code
## From PC 
cd to wherever you have the code and .pem saved
ssh -i .\AnimeMud-KeyPair.pem ubuntu@<ip_address>

### To Start MUD Service
cd to wherever you have the code and .pem saved
ssh -i .\AnimeMud-KeyPair.pem ubuntu@<ip_address>
nohup ./rom > ../../rom.log 2>&1 &

### To Stop MUD Service
cd to wherever you have the code and .pem saved
ssh -i .\AnimeMud-KeyPair.pem ubuntu@<ip_address>
ps aux | grep rom
kill <PID>

### Auto backup of players
This is done outside of AnimeMud. 
For example, setup a cron job that saves player folder to s3 using an iam role

```
sudo apt install awscli -y
aws configure
crontab -e
0 3 * * * /usr/bin/aws s3 cp --recursive /home/ubuntu/animemud/player s3://animemud-player-backup/players/ >> /home/ubuntu/player_backup.log 2>&1
```
