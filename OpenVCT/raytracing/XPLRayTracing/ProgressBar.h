#pragma once

#include <iostream>
#include <string>
#include <ctime>

class ProgressBar
{
public:
	ProgressBar()
	{
	   start();
	}
	
	void add_msg(const std::string &msg)
	{
		message = msg;
	}

	void print(const int percent)
	{
		for(int ndx=0; ndx<50; ndx++)
		{
			if      (ndx <  (percent/2))	bar.replace(ndx,1,"=");
			else if (ndx == (percent/2))	bar.replace(ndx,1,">");
			else        	              	bar.replace(ndx,1," ");			
		}
        std::cout << "\r" "[" << bar << "] ";
        std::cout.width(3);
        std::cout << percent << "%  " << elapsed() << "s " << message
                  << std::flush;
	}

	void start()
	{
		beginning = std::clock();
	}
	
	int elapsed()
	{
		current = std::clock();
		return int(double(current - beginning) / CLOCKS_PER_SEC);
	}

private:

	std::string bar;
	std::string message;
	std::clock_t beginning;
	std::clock_t current;
};
