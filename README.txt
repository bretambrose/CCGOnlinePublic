CCGOnlinePublic

For build/installation instructions, see the companion file, README_Build.txt.

I don't expect anyone to actually fork (or god forbid, develop and make pull requests on) any of my public projects here.  The primary purpose of these public projects is to serve as a public "this-is-the-sort-of-work-I-do" documentation for people or organizations who might be interested in knowing/evaluating that sort of thing.  Ultimately, all my projects are, first and foremost, learning vehicles for new subject areas as well as experiments in "better practices" in terms of developing good habits.

The eventual goal for this project is a client-server platform for some old Wizards of the Coast collectible card games (hence CCG) that my college friends and I used to love to play.  Given that this is done in my spare time with a son on the way, it will take a good number of years to finish.  I began work on this project in July 2011, and moved it from private to public repository in October 2011.


Project Goals:

Build a high-performance, heavily concurrent MMO-style platform for server-side services that is
	(1) Cross-platform (Windows, Linux)
	(2) 32 and 64 bit correct
	(3) EC2 hosted

Build a client using OpenGL.


Learning Goals:

Full persistence implementation using batched ODBC to MySQL.

Full commerical-quality network stack including serialization, error handling, concurrency, and security through encryption, hashing, and sequencing.

Learn and exercise Python to build server management scripts and cross-process testing

Exercise new features of C++0x where possible (intersection of gcc/msvc compilers)

Libraries - Boost, Loki, TBB, and CryptLib where appropriate

Patching - I know nothing about this


Software Engineering Goals:

Become better at writing unit tests

Uncompromising focus on insulation, system design, dependency minimization

Consistent, better, smarter documentation


Product Goals:

The game targets are Netrunner and Jyhad/Vampire : The Eternal Struggle.