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

#ifndef COREFERENCEDOCUMENT_H_
#define COREFERENCEDOCUMENT_H_

#include <string>
#include <vector>
#include "CoreferenceSentence.h"

class CoreferenceDocument : public Instance {
public:
  CoreferenceDocument() { conversation_ = false; }
  virtual ~CoreferenceDocument() { DeleteAllSentences(); }

  Instance* Copy() {
    CoreferenceDocument* document = new CoreferenceDocument();
    document->Initialize(name_, part_number_, sentences_);
    return static_cast<Instance*>(document);
  }

  void Initialize(const std::string &name,
                  int part_number,
                  const std::vector<CoreferenceSentence*> &sentences);

  int GetNumSentences() { return  (int)sentences_.size(); }
  CoreferenceSentence *GetSentence(int i) { return sentences_[i]; }

  const std::string &name() { return name_; }
  int part_number() { return part_number_; }
  bool is_conversation() { return conversation_; }

protected:
  void DeleteAllSentences();

protected:
  // Document name and part number.
  std::string name_;
  int part_number_;

  // True if conversation.
  bool conversation_;

  // List of sentences composing this document.
  std::vector<CoreferenceSentence*> sentences_;
};

#endif /* COREFERENCEDOCUMENT_H_*/
