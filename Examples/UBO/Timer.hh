#ifndef _H_TIMER_
#define _H_TIMER_

#include <iostream>
#include<sys/time.h>
#include <unistd.h>

/*!
  \class Timer

  \author David Rijsman

  \brief A Timer is responsible for keeping some time administration.
*/
class Timer
{
public:
  /*!
    \brief Creates an instance of Timer. Does not yet start timing!
  */
  Timer();
  /*! 
    \brief Starts the timing process.
  */
  inline void start( const char* msg = 0 );
  /*!
    \brief Ends the timing (undefined if not forst started)
  */
  inline void stop( const char* msg = 0 );
  /*!
    \brief Returns the number of seconds since the start of the timer or if ended the measured time
    between end and start
  */
  inline double getSecondsPassed( const char* msg = 0 );
  /*!
    \brief 
  */
  inline double getCurrentTime();
private:
  bool m_Running;   

  struct timeval m_Start;
  struct timeval m_End;
  struct timezone m_TimeZone;  
};

Timer::Timer():
  m_Running( false )
{
}

void Timer::start( const char* msg )
{
  if (msg) 
    std::cout << msg << std::endl;

  m_Running = true;

  gettimeofday(&m_Start, &m_TimeZone);
}

double Timer::getCurrentTime()
{
  struct timeval t;
  struct timezone tz;  

  gettimeofday(&t, &tz);

  return ( (double)t.tv_sec + (double)t.tv_usec/(1000*1000) );
}

void Timer::stop( const char* msg )
{
  if (msg) 
    std::cout << msg << std::endl;

  if( m_Running )
    {
      m_Running = false;

      gettimeofday(&m_End, &m_TimeZone);
    }
}

double Timer::getSecondsPassed( const char* msg )
{
  if (msg) 
    std::cout << msg << std::endl;

  struct timeval end;

  if( m_Running )
    {
      gettimeofday(&end, &m_TimeZone);
    }
  else
    {
      end = m_End;
    }

  double t1, t2;

  t1 =  (double)m_Start.tv_sec + (double)m_Start.tv_usec/(1000*1000);
  t2 =  (double)end.tv_sec + (double)end.tv_usec/(1000*1000);

  return t2 - t1;
}	

#endif //_H_TIMER_
