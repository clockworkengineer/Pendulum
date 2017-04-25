## [Pendulum](https://github.com/clockworkengineer/Pendulum/blob/master/Pendulum.cpp) ##

**Description**: This is a command line program to log on to an IMAP server and download e-mails from a configured mailbox, command separated mailbox list or all mailboxes for an account. A file (.eml) is created for each e-mail in a folder with the same name as its mailbox;  the files name being a combination of the mails UID/index prefix and the subject name. If configured it will poll the server every X minutes to archive any new mail. Lastly if the server disconnects it will retry the connection up to --retry times before failing with an error. 

This program is based on the code for example program ArchiveMailBox but has been re-factored heavily to enable easier future development. 
