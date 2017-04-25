## [Pendulum](https://github.com/clockworkengineer/Pendulum/blob/master/Pendulum.cpp) (E-mail Archive Program) ##

**Description**: This is a Linux command line program to log on to an IMAP server and download e-mails from a configured single mailbox, comma separated mailbox list or all mailboxes for an account. A file (.eml) is created for each e-mail in a folder with the same name as its source mailbox;  each files name being a combination of the mails UID/index prefix and the subject name. If configured it can poll the server every X minutes to download and archive any new mail. Lastly if the server disconnects it will retry the connection up to --retry times before failing with an error. 

This program is based on the code for example program ArchiveMailBox but has been re-factored heavily to enable easier future development.

**Parameters:**

    Pendulum Example Application
    Program Options:
      --help   Print help messages
      -c [ --config ] arg  	   Config File Name
      -s [ --server ] arg	   IMAP Server URL and port
      -u [ --user ] arg		   Account username
      -p [ --password ] arg	   User password
      -m [ --mailbox ] arg 	   Mailbox name (or mailbox comma separated list)
      -d [ --destination ] arg Destination folder for archived e-mail
      --poll arg   			   Poll time in minutes
      -r [ --retry ] arg  	   Server reconnect retry count
      -l [ --log ] arg		   Log file
      -u [ --updates ] 		   Search since last file archived.
      -a [ --all ] 			   Download files for all mailboxes.