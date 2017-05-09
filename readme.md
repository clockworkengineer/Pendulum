## [Pendulum](https://github.com/clockworkengineer/Pendulum/blob/master/Pendulum.cpp) (E-mail Archive Program) ##

**Description**: This is a Linux command line program to log on to an IMAP server and download e-mails from a configured single mailbox, comma separated mailbox list or all mailboxes for an account. A file (.eml) is created for each e-mail in a folder with the same name as its source mailbox;  each files name being a combination of the mails UID/index prefix and the subject name. If configured it can poll the server every X minutes to download and archive any new mail. Lastly if the server disconnects it will retry the connection up to --retry times before failing with an error. 

This program is based on the code for example program ArchiveMailBox but has been re-factored heavily to enable easier future development.

**Parameters:**

    Pendulum Email Archiver
    Program Options:
      --help   Print help messages
      -c [ --config ] arg  	   Config File Name
      -s [ --server ] arg	   IMAP Server URL and port
      -u [ --user ] arg        Account username
      -p [ --password ] arg	   User password
      -m [ --mailbox ] arg 	   Mailbox name (or mailbox comma separated list)
      -d [ --destination ] arg Destination folder for archived e-mail
      --poll arg               Poll time in minutes
      -r [ --retry ] arg       Server reconnect retry count
      -l [ --log ] arg         Log file
      -i [ --ignore ] arg      Ignore mailbox list
      -u [ --updates ]         Search since last file archived.
      -a [ --all ]             Download files for all mailboxes.

## To Do List ##

1. Addition of mailbox ignore list for use with –all so that for instance Deleted/Junk/Spam mailboxes are not archived.
1. The ability to work with multiple servers
1. Use of threads.
1. Qt based UI (Know little about Qt but it looks like the best choice for a portable/flexible way to provide a cross platform user interface).
