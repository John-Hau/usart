Serial Programming HOWTO
Prev		Next
3. Program Examples
All examples have been derived from miniterm.c. The type ahead buffer is limited to 255 characters, just like the maximum string length for canonical input processing (<linux/limits.h> or <posix1_lim.h>).

See the comments in the code for explanation of the use of the different input modes. I hope that the code is understandable. The example for canonical input is commented best, the other examples are commented only where they differ from the example for canonical input to emphasize the differences.

The descriptions are not complete, but you are encouraged to experiment with the examples to derive the best solution for your application.

Don't forget to give the appropriate serial ports the right permissions (e. g.: chmod a+rw /dev/ttyS1)!

3.1. Canonical Input Processing
        #include <sys/types.h>
        #include <sys/stat.h>
        #include <fcntl.h>
        #include <termios.h>
        #include <stdio.h>

        /* baudrate settings are defined in <asm/termbits.h>, which is
        included by <termios.h> */
        #define BAUDRATE B38400            
        /* change this definition for the correct port */
        #define MODEMDEVICE "/dev/ttyS1"
        #define _POSIX_SOURCE 1 /* POSIX compliant source */

        #define FALSE 0
        #define TRUE 1

        volatile int STOP=FALSE; 

        main()
        {
          int fd,c, res;
          struct termios oldtio,newtio;
          char buf[255];
        /* 
          Open modem device for reading and writing and not as controlling tty
          because we don't want to get killed if linenoise sends CTRL-C.
        */
         fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY ); 
         if (fd <0) {perror(MODEMDEVICE); exit(-1); }
        
         tcgetattr(fd,&oldtio); /* save current serial port settings */
         bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
        
        /* 
          BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
          CRTSCTS : output hardware flow control (only used if the cable has
                    all necessary lines. See sect. 7 of Serial-HOWTO)
          CS8     : 8n1 (8bit,no parity,1 stopbit)
          CLOCAL  : local connection, no modem contol
          CREAD   : enable receiving characters
        */
         newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
         
        /*
          IGNPAR  : ignore bytes with parity errors
          ICRNL   : map CR to NL (otherwise a CR input on the other computer
                    will not terminate input)
          otherwise make device raw (no other input processing)
        */
         newtio.c_iflag = IGNPAR | ICRNL;
         
        /*
         Raw output.
        */
         newtio.c_oflag = 0;
         
        /*
          ICANON  : enable canonical input
          disable all echo functionality, and don't send signals to calling program
        */
         newtio.c_lflag = ICANON;
         
        /* 
          initialize all control characters 
          default values can be found in /usr/include/termios.h, and are given
          in the comments, but we don't need them here
        */
         newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
         newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
         newtio.c_cc[VERASE]   = 0;     /* del */
         newtio.c_cc[VKILL]    = 0;     /* @ */
         newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
         newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
         newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
         newtio.c_cc[VSWTC]    = 0;     /* '\0' */
         newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
         newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
         newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
         newtio.c_cc[VEOL]     = 0;     /* '\0' */
         newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
         newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
         newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
         newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
         newtio.c_cc[VEOL2]    = 0;     /* '\0' */
        
        /* 
          now clean the modem line and activate the settings for the port
        */
         tcflush(fd, TCIFLUSH);
         tcsetattr(fd,TCSANOW,&newtio);
        
        /*
          terminal settings done, now handle input
          In this example, inputting a 'z' at the beginning of a line will 
          exit the program.
        */
         while (STOP==FALSE) {     /* loop until we have a terminating condition */
         /* read blocks program execution until a line terminating character is 
            input, even if more than 255 chars are input. If the number
            of characters read is smaller than the number of chars available,
            subsequent reads will return the remaining chars. res will be set
            to the actual number of characters actually read */
            res = read(fd,buf,255); 
            buf[res]=0;             /* set end of string, so we can printf */
            printf(":%s:%d\n", buf, res);
            if (buf[0]=='z') STOP=TRUE;
         }
         /* restore the old port settings */
         tcsetattr(fd,TCSANOW,&oldtio);
        }

      
3.2. Non-Canonical Input Processing
In non-canonical input processing mode, input is not assembled into lines and input processing (erase, kill, delete, etc.) does not occur. Two parameters control the behavior of this mode: c_cc[VTIME] sets the character timer, and c_cc[VMIN] sets the minimum number of characters to receive before satisfying the read.

If MIN > 0 and TIME = 0, MIN sets the number of characters to receive before the read is satisfied. As TIME is zero, the timer is not used.

If MIN = 0 and TIME > 0, TIME serves as a timeout value. The read will be satisfied if a single character is read, or TIME is exceeded (t = TIME *0.1 s). If TIME is exceeded, no character will be returned.

If MIN > 0 and TIME > 0, TIME serves as an inter-character timer. The read will be satisfied if MIN characters are received, or the time between two characters exceeds TIME. The timer is restarted every time a character is received and only becomes active after the first character has been received.

If MIN = 0 and TIME = 0, read will be satisfied immediately. The number of characters currently available, or the number of characters requested will be returned. According to Antonino (see contributions), you could issue a fcntl(fd, F_SETFL, FNDELAY); before reading to get the same result.

By modifying newtio.c_cc[VTIME] and newtio.c_cc[VMIN] all modes described above can be tested.

      #include <sys/types.h>
      #include <sys/stat.h>
      #include <fcntl.h>
      #include <termios.h>
      #include <stdio.h>
        
      #define BAUDRATE B38400
      #define MODEMDEVICE "/dev/ttyS1"
      #define _POSIX_SOURCE 1 /* POSIX compliant source */
      #define FALSE 0
      #define TRUE 1
        
      volatile int STOP=FALSE; 
       
      main()
      {
        int fd,c, res;
        struct termios oldtio,newtio;
        char buf[255];
        
        fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY ); 
        if (fd <0) {perror(MODEMDEVICE); exit(-1); }
        
        tcgetattr(fd,&oldtio); /* save current port settings */
        
        bzero(&newtio, sizeof(newtio));
        newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;
        
        /* set input mode (non-canonical, no echo,...) */
        newtio.c_lflag = 0;
         
        newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
        newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */
        
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd,TCSANOW,&newtio);
        
        
        while (STOP==FALSE) {       /* loop for input */
          res = read(fd,buf,255);   /* returns after 5 chars have been input */
          buf[res]=0;               /* so we can printf... */
          printf(":%s:%d\n", buf, res);
          if (buf[0]=='z') STOP=TRUE;
        }
        tcsetattr(fd,TCSANOW,&oldtio);
      }
    
3.3. Asynchronous Input
      #include <termios.h>
      #include <stdio.h>
      #include <unistd.h>
      #include <fcntl.h>
      #include <sys/signal.h>
      #include <sys/types.h>
        
      #define BAUDRATE B38400
      #define MODEMDEVICE "/dev/ttyS1"
      #define _POSIX_SOURCE 1 /* POSIX compliant source */
      #define FALSE 0
      #define TRUE 1
        
      volatile int STOP=FALSE; 
        
      void signal_handler_IO (int status);   /* definition of signal handler */
      int wait_flag=TRUE;                    /* TRUE while no signal received */
        
      main()
      {
        int fd,c, res;
        struct termios oldtio,newtio;
        struct sigaction saio;           /* definition of signal action */
        char buf[255];
        
        /* open the device to be non-blocking (read will return immediatly) */
        fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (fd <0) {perror(MODEMDEVICE); exit(-1); }
        
        /* install the signal handler before making the device asynchronous */
        saio.sa_handler = signal_handler_IO;
        saio.sa_mask = 0;
        saio.sa_flags = 0;
        saio.sa_restorer = NULL;
        sigaction(SIGIO,&saio,NULL);
          
        /* allow the process to receive SIGIO */
        fcntl(fd, F_SETOWN, getpid());
        /* Make the file descriptor asynchronous (the manual page says only 
           O_APPEND and O_NONBLOCK, will work with F_SETFL...) */
        fcntl(fd, F_SETFL, FASYNC);
        
        tcgetattr(fd,&oldtio); /* save current port settings */
        /* set new port settings for canonical input processing */
        newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR | ICRNL;
        newtio.c_oflag = 0;
        newtio.c_lflag = ICANON;
        newtio.c_cc[VMIN]=1;
        newtio.c_cc[VTIME]=0;
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd,TCSANOW,&newtio);
         
        /* loop while waiting for input. normally we would do something
           useful here */ 
        while (STOP==FALSE) {
          printf(".\n");usleep(100000);
          /* after receiving SIGIO, wait_flag = FALSE, input is available
             and can be read */
          if (wait_flag==FALSE) { 
            res = read(fd,buf,255);
            buf[res]=0;
            printf(":%s:%d\n", buf, res);
            if (res==1) STOP=TRUE; /* stop loop if only a CR was input */
            wait_flag = TRUE;      /* wait for new input */
          }
        }
        /* restore old port settings */
        tcsetattr(fd,TCSANOW,&oldtio);
      }
        
      /***************************************************************************
      * signal handler. sets wait_flag to FALSE, to indicate above loop that     *
      * characters have been received.                                           *
      ***************************************************************************/
        
      void signal_handler_IO (int status)
      {
        printf("received SIGIO signal.\n");
        wait_flag = FALSE;
      }
    
3.4. Waiting for Input from Multiple Sources
This section is kept to a minimum. It is just intended to be a hint, and therefore the example code is kept short. This will not only work with serial ports, but with any set of file descriptors.

The select call and accompanying macros use a fd_set. This is a bit array, which has a bit entry for every valid file descriptor number. select will accept a fd_set with the bits set for the relevant file descriptors and returns a fd_set, in which the bits for the file descriptors are set where input, output, or an exception occurred. All handling of fd_set is done with the provided macros. See also the manual page select(2).

      #include <sys/time.h>
      #include <sys/types.h>
      #include <unistd.h>
        
      main()
      {
        int    fd1, fd2;  /* input sources 1 and 2 */
        fd_set readfs;    /* file descriptor set */
        int    maxfd;     /* maximum file desciptor used */
        int    loop=1;    /* loop while TRUE */ 
        
        /* open_input_source opens a device, sets the port correctly, and
           returns a file descriptor */
        fd1 = open_input_source("/dev/ttyS1");   /* COM2 */
        if (fd1<0) exit(0);
        fd2 = open_input_source("/dev/ttyS2");   /* COM3 */
        if (fd2<0) exit(0);
        maxfd = MAX (fd1, fd2)+1;  /* maximum bit entry (fd) to test */
        
        /* loop for input */
        while (loop) {
          FD_SET(fd1, &readfs);  /* set testing for source 1 */
          FD_SET(fd2, &readfs);  /* set testing for source 2 */
          /* block until input becomes available */
          select(maxfd, &readfs, NULL, NULL, NULL);
          if (FD_ISSET(fd1))         /* input from source 1 available */
            handle_input_from_source1();
          if (FD_ISSET(fd2))         /* input from source 2 available */
            handle_input_from_source2();
        }
      }   
    
The given example blocks indefinitely, until input from one of the sources becomes available. If you need to timeout on input, just replace the select call by:
        int res;
        struct timeval Timeout;

        /* set timeout value within input loop */
        Timeout.tv_usec = 0;  /* milliseconds */
        Timeout.tv_sec  = 1;  /* seconds */
        res = select(maxfd, &readfs, NULL, NULL, &Timeout);
        if (res==0)
        /* number of file descriptors with input = 0, timeout occurred. */ 
      
This example will timeout after 1 second. If a timeout occurs, select will return 0, but beware that Timeout is decremented by the time actually waited for input by select. If the timeout value is zero, select will return immediatly.

Prev	Home	Next
Getting started	