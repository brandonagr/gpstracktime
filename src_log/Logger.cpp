#include "Logger.h"
#include <fstream>

using namespace std;

//=========================================================================
GPSLogger::GPSLogger(std::string params_filename)
:params_(params_filename),
 gps_(params_)
{
  //anything else to set up?
  
  gps_lines_.reserve(10000);
}


void GPSLogger::run_logging()
{
  cout<<"Waiting to start logging..."<<endl;

  LARGE_INTEGER freq_, prev_time_, this_time_;   

  double dt,prev_dt;
  double time_since_fix=0;
  QueryPerformanceFrequency(&freq_);
  double freq_inv_=1/(double)freq_.QuadPart;
  QueryPerformanceCounter(&prev_time_);  

  prev_dt=0;

  bool in_session=false;


  while(true)
  {
    QueryPerformanceCounter(&this_time_); //calc dt
    dt = double(this_time_.QuadPart-prev_time_.QuadPart) * freq_inv_;
    prev_time_=this_time_;
    time_since_fix+=dt;

    if (time_since_fix>10.0) //dead man's timer expired, tell user something is up
    {
      cout<<endl<<endl<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl<<"Hopelessly lost GPS fix... You need to fix something!"<<endl;

      if (dump_data())
        cout<<"Well some data was saved, but program is about to terminate..."<<endl;
        
      string wait;
      getline(cin,wait);
      exit(0);
    }    

    string linedata=gps_.get_gps_line(dt);    
    if (linedata.size()>0 && GPSData::is_GPRMC(linedata)) //new position update has come in
    {
      GPSData gpspos_=GPSData(linedata);

      cout<<gpspos_<<endl;
      
      if (gpspos_.valid_)
      {
        time_since_fix=0.0; //reset dead mans timer

        if (in_session)
        {
          //make sure speed is still above 35
          if (gpspos_.speed_<10.0)
          {
            cout<<"no longer in session!"<<endl;
            in_session=false;

            if (dump_data())
              cout<<"dumped content to file!"<<endl;          
          }
          else
          {
            stringstream out;
            out<<gpspos_;            
            gps_lines_.push_back(out.str());
          }
        }
        else
        {
          if (gpspos_.speed_>50.0)
          {
            cout<<"going into session!"<<endl;
            in_session=true;
          }
        }          
      }
      else
      {
        cout<<"Lost GPS fix!...."<<endl;
      }
    }    

    //perform loop timing to regulate how fast it goes
    if (dt<0.1)
    {
      int t=91-int(((dt+prev_dt)/2.0f)*1000);

      if (t>1)
        Sleep(t); //stall some time, avg previous two frame times in order to prevent oscillating between .2 and .8 or similiar
    }
    
    prev_dt=dt;
  }
}

bool GPSLogger::dump_data()
{
  if (gps_lines_.size()==0)
    return false;
  std::string file(params_.get<std::string>("GPSSaveFile"));
  ofstream out(file.c_str(),ios::app);

  for(vector<string>::const_iterator i=gps_lines_.begin(); i!=gps_lines_.end(); i++)
  {
    out<<*i<<endl;
  }

  gps_lines_.clear();

  return true;
}