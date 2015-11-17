// Copyright (c) 2012-2015 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.3.
//
// TurboParser 2.3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.3.  If not, see <http://www.gnu.org/licenses/>.

#ifndef COREFERENCEDETERMINER_H_
#define COREFERENCEDETERMINER_H_

// TODO(atm): there is a lot of redundancy here with CoreferenceDeterminer.
// We should refactor this some time later.
struct CoreferenceDeterminerNumber {
  enum types {
    SINGULAR = 0,
    PLURAL,
    UNDEFINED,
    COUNT
  };
};

struct CoreferenceDeterminerGender {
  enum types {
    MALE = 0,
    FEMALE,
    NEUTRAL,
    UNDEFINED,
    COUNT
  };
};

class CoreferenceDeterminer {
public:
  CoreferenceDeterminer() { ClearFlags(); }
  CoreferenceDeterminer(const std::string &code_flags) { SetFlags(code_flags); }
  virtual ~CoreferenceDeterminer() {}

  void Save(FILE *fs) {
    bool success;
    success = WriteUINT8(fs, number_flag_);
    CHECK(success);
    success = WriteUINT8(fs, gender_flag_);
    CHECK(success);
  }

  void Load(FILE *fs) {
    bool success;
    success = ReadUINT8(fs, &number_flag_);
    CHECK(success);
    success = ReadUINT8(fs, &gender_flag_);
    CHECK(success);
  }

  uint8_t number_flag() { return number_flag_; }
  uint8_t gender_flag() { return gender_flag_; }

  void ClearFlags() {
    number_flag_ = 0x0;
    gender_flag_ = 0x0;
  }

  void SetFlags(const std::string &code_flags) {
    CHECK_EQ(code_flags.length(), 2);

    ClearFlags();
    char ch = code_flags[0]; // Number flag.
    if (ch == 's') {
      SetNumberSingular();
    } else if (ch == 'p') {
      SetNumberPlural();
    } else if (ch == 'x') {
      SetNumberUndefined();
    } else {
      CHECK(false) << "Invalid number flag: " << ch;
    }

    ch = code_flags[1]; // Gender flag.
    if (ch == 'm') {
      SetGenderMale();
    } else if (ch == 'f') {
      SetGenderFemale();
    } else if (ch == 'n') {
      SetGenderNeutral();
    } else if (ch == 'x') {
      SetGenderUndefined();
    } else {
      CHECK(false) << "Invalid gender flag: " << ch;
    }
  }

public:
  bool IsNumberSingular() {
    return number_flag_ & (0x1 << CoreferenceDeterminerNumber::SINGULAR);
  }
  bool IsNumberPlural() {
    return number_flag_ & (0x1 << CoreferenceDeterminerNumber::PLURAL);
  }
  bool IsNumberUndefined() {
    return number_flag_ & (0x1 << CoreferenceDeterminerNumber::UNDEFINED);
  }
  bool IsGenderMale() {
    return gender_flag_ & (0x1 << CoreferenceDeterminerGender::MALE);
  }
  bool IsGenderFemale() {
    return gender_flag_ & (0x1 << CoreferenceDeterminerGender::FEMALE);
  }
  bool IsGenderNeutral() {
    return gender_flag_ & (0x1 << CoreferenceDeterminerGender::NEUTRAL);
  }
  bool IsGenderUndefined() {
    return gender_flag_ & (0x1 << CoreferenceDeterminerGender::UNDEFINED);
  }

  void SetNumberSingular() {
    number_flag_ |= (0x1 << CoreferenceDeterminerNumber::SINGULAR);
  }
  void SetNumberPlural() {
    number_flag_ |= (0x1 << CoreferenceDeterminerNumber::PLURAL);
  }
  void SetNumberUndefined() {
    number_flag_ |= (0x1 << CoreferenceDeterminerNumber::UNDEFINED);
  }
  void SetGenderMale() {
    gender_flag_ |= (0x1 << CoreferenceDeterminerGender::MALE);
  }
  void SetGenderFemale() {
    gender_flag_ |= (0x1 << CoreferenceDeterminerGender::FEMALE);
  }
  void SetGenderNeutral() {
    gender_flag_ |= (0x1 << CoreferenceDeterminerGender::NEUTRAL);
  }
  void SetGenderUndefined() {
    gender_flag_ |= (0x1 << CoreferenceDeterminerGender::UNDEFINED);
  }

protected:
  uint8_t number_flag_;
  uint8_t gender_flag_;
};

#endif /* COREFERENCEDETERMINER_H_ */
