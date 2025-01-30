#ifndef BITCOMP_HPP
#define BITCOMP_HPP

#ifdef _WIN32
#pragma warning (disable: 4251) // http://msdn2.microsoft.com/en-gb/library/esew7y1w.aspx
#pragma warning (disable: 4996)
#endif 


#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <exception>

namespace bitcomp {

struct Level {
  enum Enum {
    undefined,
    input,
    output_0,
    output_1,
    output_z
  };
};

struct Type {
  enum Enum {
    undefined,
    input,
    output
  };
};

struct Technology {
  enum Enum {
    undefined,
    ttl,
    cmos
  };
};


class Chip {
public:
  Chip();
  virtual ~Chip();

  virtual size_t size() const=0;
  virtual void voltage(double& min, double& max) const;
  virtual Technology::Enum technology() const;
  virtual Type::Enum type(size_t) const;
  virtual size_t fan_out(size_t, Technology::Enum) const;

  virtual void set(size_t, Level::Enum, double time);
  virtual Level::Enum get(size_t, double time) const;
};

class M23K640 : public Chip {
/* 8-pin 64 Kibit serial SRAM
 * 1  /CS
 * 2  SO
 * 3  -
 * 4  Vss (Ground)
 * 5  SI
 * 6  SCK
 * 7  /HOLD
 * 8  Vcc  (2.7-3.6 V)
 */
};

class HCF4517B : public Chip {
/* 16-pin dual 64-stage static shift register
 *  1   Q16(A)
 *  2   Q48(A)
 *  3   WE(A)
 *  4   CL(A)
 *  5   Q64(A)
 *  6   Q32(A)
 *  7   D(A)
 *  8   Vss (Ground)
 *  9   D(B)
 *  10  Q32(B)
 *  11  Q64(B)
 *  12  CL(B)
 *  13  WE(B)
 *  14  Q48(B)
 *  15  Q16(B)
 *  16  Vdd (3-15 V)
 */
};

class HEF4557B : public Chip {
/* 16-pin 1-to-64 bit variable length shift register
 *  1   L2
 *  2   L1
 *  3   MR
 *  4   CP0
 *  5   /CP1
 *  6   DB
 *  7   DA
 *  8   Vss
 *  9   A//B
 *  10  Q
 *  11  /Q
 *  12  L32
 *  13  L16
 *  14  L8
 *  15  L4
 *  16  Vdd (3-15 V)
 */
};

class HCF40193B : public Chip {
/* 16-pin presettable up/down counters
 *  1   J2
 *  2   Q2
 *  3   Q1
 *  4   CLOCK DOWN
 *  5   CLOCK UP
 *  6   Q3
 *  7   Q4
 *  8   Vss
 *  9   J4
 *  10  J3
 *  11  /(PRESET ENABLE)
 *  12  /CARRY
 *  13  /BORROW
 *  14  RESET
 *  15  J1
 *  16  Vdd (3-15 V)
 */
 };

 class MC14516B : public Chip {
 /* 16-pin presettable up/down counters
 *  1   PE
 *  2   Q3
 *  3   P3
 *  4   P0
 *  5   /(CARRY IN)
 *  6   Q0
 *  7   /(CARRY OUT)
 *  8   Vss
 *  9   R
 *  10  U/D
 *  11  Q1
 *  12  P1
 *  13  P2
 *  14  Q2
 *  15  C
 *  16  Vdd (3-15 V)
 */
 };

 class HEF4512B : public Chip {
/* 16-pin 8-input multiplexer with 3-state output
 *  1   I0
 *  2   I1
 *  3   I2
 *  4   I3
 *  5   I4
 *  6   I5
 *  7   I6
 *  8   Vss
 *  9   I7
 *  10  /E
 *  11  S0
 *  12  S1
 *  13  S2
 *  14  O
 *  15  /EO
 *  16  Vdd
 */
 };

 class CD4094 : public Chip {
/* 16-pin 8-stage shift-and-store bus register
 *  1   Strobe
 *  2   Data
 *  3   Clock
 *  4   Q1
 *  5   Q2
 *  6   Q3
 *  7   Q4
 *  8   Vss
 *  9   QS
 *  10  Q'S
 *  11  Q8
 *  12  Q7
 *  13  Q6
 *  14  Q5
 *  15  Output Enable
 *  16  Vdd
 */
 };

class Network {
  std::vector<Chip*>    chip_list_;
  std::vector<size_t>   pin_list_;
  Level::Enum           value_;
};

}

#endif
