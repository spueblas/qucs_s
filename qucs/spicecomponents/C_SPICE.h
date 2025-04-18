/***************************************************************************
                         C_SPICE.h  -  description
                   --------------------------------------
    begin                    : Fri Mar 9 2007
    copyright              : (C) 2007 by Gunther Kraut
    email                     : gn.kraut@t-online.de
    spice4qucs code added  Fri. 2 March 2015
    copyright              : (C) 2015 by Mike Brinson
    email                    : mbrin72043@yahoo.co.uk

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 #ifndef C_SPICE_H
#define C_SPICE_H

#include "components/component.h"

class C_SPICE: public MultiViewComponent {
public:
  C_SPICE();
  ~C_SPICE();
  Component* newOne();
  static Element* info(QString&, char* &, bool getNewOne=false);
  static Element* info_C3(QString&, char* &, bool getNewOne=false);

protected:
  void createSymbol();
  QString netlist();
  QString spice_netlist(spicecompat::SpiceDialect dialect = spicecompat::SPICEDefault);
  virtual QString cdl_netlist();
};

#endif // C_SPICE_H
