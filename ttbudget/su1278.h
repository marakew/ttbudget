
/*
 */

#ifndef _su1278_h_
#define _su1278_h_

class CDvbAdapter;
class CTunerSU1278 : public CTuner
{
protected:

public:
	CTunerSU1278(CDvbAdapter *dvb);
	~CTunerSU1278();

};
#endif

