#include "mPower.h"

bool mPower::readUntil(unsigned char wait){
  unsigned long timeout = millis() + 1000;
  while (timeout > millis()) {
    if(m_device.available()){
      char tChar = m_device.read();
      if(tChar == wait){
        return true;
      }
    }
  }
  return false;
}

bool mPower::login(){
    if (!m_device.connect(m_host, TELNET_PORT)) 
        return false;

    if(!readUntil(':')) return false;
    m_device.printf("%s\n",m_user);

    if(!readUntil(':')) return false;
    m_device.printf("%s\n",m_password);  

    if(!readUntil('#')) return false;

    // this gehts the defauld command line prompt
    m_device.printf("\n");  
    m_device.readStringUntil('\n');
    m_cmdLine = m_device.readStringUntil(' ');

    return true;
}

bool mPower::getPort(unsigned char port){
    getStatus();
    return enabled[port];
}

bool mPower::setPort(unsigned char port, bool status){
    if(!login())
        return false;

    m_device.printf("echo %c > /proc/power/relay%c\n",(status==true?'1':'0'),'1'+port);
    readUntil('\n');
    m_device.stop();
    return true;
}

bool mPower::setPorts(){
    if(!portsAvailable)
        getStatus();

    if(!login())
        return false;
    for(unsigned char port=0; port < portsAvailable - 1; port++){
        m_device.printf("echo %c > /proc/power/relay%c\n",(enabled[port]==true?'1':'0'),'1'+port);
        readUntil('\n');
    }
    m_device.stop();
}


bool mPower::getStatus(){
    String tLine;

    if(!login())
        return false;

    m_device.printf("cat /proc/power/relay*\n");
    // drop first new line
    if(!readUntil('\n')) return false;
    for(int port = 0; port <6; port ++){
        tLine = m_device.readStringUntil('\n');
        if(tLine.startsWith(m_cmdLine))
            break;
        enabled[port] = (tLine[0] == '1');
        portsAvailable = port + 1;
    }

    m_device.printf("cat /proc/power/active_pwr*\n");
    // drop first new line
    if(!readUntil('\n')) return false;
    for(int port = 0; port <6; port ++){
        tLine = m_device.readStringUntil('\n');
        if(tLine.startsWith(m_cmdLine))
            break;
        sscanf (tLine.c_str(),"%f",&power[port]); 
    }

    #ifdef POWER_FACTOR
    m_device.printf("cat /proc/power/pf*\n");
    // drop first new line
    if(!readUntil('\n')) return false;
    for(int port = 0; port <6; port ++){
        tLine = m_device.readStringUntil('\n');
        if(tLine.startsWith(m_cmdLine))
            break;
        sscanf (tLine.c_str(),"%f",&powerFactor[port]); 
    }
    #endif
    m_device.stop();
    return true;

}
