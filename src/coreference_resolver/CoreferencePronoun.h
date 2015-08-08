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

#ifndef COREFERENCEPRONOUN_H_
#define COREFERENCEPRONOUN_H_

struct CoreferencePronounPerson {
  enum types {
    FIRST = 0,
    SECOND,
    THIRD,
    UNDEFINED,
    COUNT
  };
};

struct CoreferencePronounNumber {
  enum types {
    SINGULAR = 0,
    PLURAL,
    UNDEFINED,
    COUNT
  };
};

struct CoreferencePronounGender {
  enum types {
    MALE = 0,
    FEMALE,
    NEUTRAL,
    UNDEFINED,
    COUNT
  };
};

class CoreferencePronoun {
 public:
  CoreferencePronoun() { ClearFlags(); }
  CoreferencePronoun(const std::string &code_flags) { SetFlags(code_flags); }
  virtual ~CoreferencePronoun() {}

  void ClearFlags() {
    person_flag_ = 0x0;
    number_flag_ = 0x0;
    gender_flag_ = 0x0;
  }

  void SetFlags(const std::string &code_flags) {
    CHECK_EQ(code_flags.length(), 3);

    ClearFlags();
    char ch = code_flags[0]; // Person flag.
    if (ch == '1') {
      SetPersonFirst();
    } else if (ch == '2') {
      SetPersonSecond();
    } else if (ch == '3') {
      SetPersonThird();
    } else if (ch == 'x') {
      SetPersonUndefined();
    } else {
      CHECK(false) << "Invalid person flag: " << ch;
    }

    ch = code_flags[1]; // Number flag.
    if (ch == 's') {
      SetNumberSingular();
    } else if (ch == 'p') {
      SetNumberPlural();
    } else if (ch == 'x') {
      SetNumberUndefined();
    } else {
      CHECK(false) << "Invalid number flag: " << ch;
    }

    ch = code_flags[2]; // Gender flag.
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
  bool IsPersonFirst() {
    return person_flag_ & (0x1 << CoreferencePronounPerson::FIRST);
  }
  bool IsPersonSecond() {
    return person_flag_ & (0x1 << CoreferencePronounPerson::SECOND);
  }
  bool IsPersonThird() {
    return person_flag_ & (0x1 << CoreferencePronounPerson::THIRD);
  }
  bool IsPersonUndefined() {
    return person_flag_ & (0x1 << CoreferencePronounPerson::UNDEFINED);
  }
  bool IsNumberSingular() {
    return number_flag_ & (0x1 << CoreferencePronounNumber::SINGULAR);
  }
  bool IsNumberPlural() {
    return number_flag_ & (0x1 << CoreferencePronounNumber::PLURAL);
  }
  bool IsNumberUndefined() {
    return number_flag_ & (0x1 << CoreferencePronounNumber::UNDEFINED);
  }
  bool IsGenderMale() {
    return gender_flag_ & (0x1 << CoreferencePronounGender::MALE);
  }
  bool IsGenderFemale() {
    return gender_flag_ & (0x1 << CoreferencePronounGender::FEMALE);
  }
  bool IsGenderNeutral() {
    return gender_flag_ & (0x1 << CoreferencePronounGender::NEUTRAL);
  }
  bool IsGenderUndefined() {
    return gender_flag_ & (0x1 << CoreferencePronounGender::UNDEFINED);
  }

  void SetPersonFirst() {
    person_flag_ |= (0x1 << CoreferencePronounPerson::FIRST);
  }
  void SetPersonSecond() {
    person_flag_ |= (0x1 << CoreferencePronounPerson::SECOND);
  }
  void SetPersonThird() {
    person_flag_ |= (0x1 << CoreferencePronounPerson::THIRD);
  }
  void SetPersonUndefined() {
    person_flag_ |= (0x1 << CoreferencePronounPerson::UNDEFINED);
  }
  void SetNumberSingular() {
    number_flag_ |= (0x1 << CoreferencePronounNumber::SINGULAR);
  }
  void SetNumberPlural() {
    number_flag_ |= (0x1 << CoreferencePronounNumber::PLURAL);
  }
  void SetNumberUndefined() {
    number_flag_ |= (0x1 << CoreferencePronounNumber::UNDEFINED);
  }
  void SetGenderMale() {
    gender_flag_ |= (0x1 << CoreferencePronounGender::MALE);
  }
  void SetGenderFemale() {
    gender_flag_ |= (0x1 << CoreferencePronounGender::FEMALE);
  }
  void SetGenderNeutral() {
    gender_flag_ |= (0x1 << CoreferencePronounGender::NEUTRAL);
  }
  void SetGenderUndefined() {
    gender_flag_ |= (0x1 << CoreferencePronounGender::UNDEFINED);
  }

 protected:
  uint8_t person_flag_;
  uint8_t number_flag_;
  uint8_t gender_flag_;
};

#endif /* COREFERENCEPRONOUN_H_ */
