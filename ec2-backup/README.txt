EC2-BACKUP(1)		  BSD General Commands Manual		 EC2-BACKUP(1)

NAME
     ec2-backup -- backup a directory into Elastic Block Storage (EBS)

SYNOPSIS
     ec2-backup [-h] [-l filter] [-r filter] [-v volume-id] dir

DESCRIPTION
     The ec2-backup tool performs a backup of the given directory into Amazon
     Elastic Block Storage (EBS).  This is achieved by creating a volume of
     the appropriate size, attaching it to an EC2 instance and finally copying
     the files from the given directory onto this volume.

OPTIONS
     ec2-backup accepts the following command-line flags:

     -h 	   Print a usage statement and exit.

     -l filter	   Pass data through the given filter command on the local
		   host before copying the data to the remote system.

     -r filter	   Pass data through the given filter command on the remote
		   host before writing the data to the volume.

     -v volume-id  Use the given volume instead of creating a new one.

DETAILS
     ec2-backup will perform a backup of the given directory to an ESB volume.
     The backup is done with the help of the tar(1) command on the local host,
     writing the resulting archive directly to the block device.  That is,
     ec2-backup does not create or use a filesystem on the volume.  Instead,
     ec2-backup utilizes the dd(1) command to write out the data to the raw
     volume.  In essence, ec2-backup wraps the following pipeline:

	   tar cf - <dir> | ssh ec2-instance "dd of=/dev/sda3"

     ec2-backup does not use any temporary files, nor creates a local copy of
     the archive it writes to the volume.

     ec2-backup can pass the archive it creates through a filter command on
     either the local or the remote host.  This allows the user to e.g. per-
     form encryption of the backup.

     Unless the -v flag is specified, ec2-backup will create a new volume, the
     size of which will be at least two times the size of the directory to be
     backed up.

     ec2-backup will create an instance suitable to perform the backup, attach
     the volume in question and then back up the data from the given direc-
     tory.  Afterwards, ec2-backup will terminate the instance it created.

OUTPUT
     If successful, ec2-backup will print the volume-id of the volume to which
     it backed up the data as the only output.

     Unless the EC2_BACKUP_VERBOSE environment variable is set, ec2-backup
     will not generate any other output unless any errors are encountered.  If
     that variable is set, it may print out some useful information about what
     steps it is currently performing.

     Any errors encountered cause a meaningful error message to be printed to
     STDERR.

ENVIRONMENT
     ec2-backup assumes that the user has set up their environment for general
     use with the EC2 tools.  That is, it will not set or modify the variables
     AWS_CONFIG_FILE, EC2_CERT, EC2_HOME or EC2_PRIVATE_KEY.

     ec2-backup allows the user to add custom flags to the commands related to
     starting a new EC2 instance via the EC2_BACKUP_FLAGS_AWS environment
     variable.

     ec2-backup also assumes that the user has set up their ~/.ssh/config file
     to access instances in EC2 via ssh(1) without any additional settings.
     It does allow the user to add custom flags to the ssh(1) commands it
     invokes via the EC2_BACKUP_FLAGS_SSH environment variable.

     As noted above, the EC2_BACKUP_VERBOSE variable may cause ec2-backup to
     generate informational output as it runs.

EXIT STATUS
     The ec2-backup will exit with a return status of 0 under normal circum-
     stances.  If an error occurred, ec2-backup will exit with a value >0.

EXAMPLES
     The following examples illustrate common usage of this tool.

     To back up the entire filesystem:

	   $ ec2-backup /
	   vol-1a2b3c4d

     To create a complete backup of the current working directory using
     defaults to the volume with the ID vol-1a2b3c4d, possibly overwriting any
     data previously stored there:

	   $ ec2-backup -v vol-1a2b3c4d .
	   vol-1a2b3c4d

     Suppose a user has their ~/.ssh/config set up to use the private key
     ~/.ec2/stevens but wishes to use the key ~/.ssh/ec2-key instead:

	   $ export EC2_BACKUP_FLAGS_SSH="-i ~/.ssh/ec2-key"
	   $ ec2-backup .
	   vol-1a2b3c4d

     To force creation of an instance type of t1.micro instead of whatever
     defaults might apply

	   $ export EC2_BACKUP_FLAGS_AWS="--instance-type t1.micro"
	   $ ec2-backup .
	   vol-1a2b3c4d

     To locally encrypt the backup of the '/var/secrets' directory:

	   $ ec2-backup -l 'gpg -e -r 9BED3DD7' /var/secrets
	   vol-1a2b3c4d

     The same as above, but perform encryption on the remote system:

	   $ ec2-backup -r 'gpg -e -r 9BED3DD7' /var/secrets
	   vol-1a2b3c4d

SEE ALSO
     aws help, cat(1), dd(1), ssh(1), tar(1)

HISTORY
     ec2-backup was originally assigned by Jan Schaumann
     <jschauma@cs.stevens.edu> as a homework assignment for the class "Aspects
     of System Administration" at Stevens Institute of Technology in the
     Spring of 2011.

BSD				April 30, 2018				   BSD
