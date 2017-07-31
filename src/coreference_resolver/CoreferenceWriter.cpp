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

#include "CoreferenceWriter.h"
#include "CoreferenceDocument.h"
#include <iostream>
#include <sstream>

void CoreferenceWriter::Write(Instance *instance) {
  CoreferenceDocument *document = static_cast<CoreferenceDocument*>(instance);
  char part_number[4];

#ifdef _WIN32
  _snprintf(part_number, sizeof(part_number), "%03d", document->part_number());
#else
  snprintf(part_number, sizeof(part_number), "%03d", document->part_number());
#endif

  //std::cout << "Document: " << document->name() << std::endl;

  os_ << "#begin document (" << document->name() << "); part "
    << part_number << std::endl;
  for (int i = 0; i < document->GetNumSentences(); ++i) {
    // Start writing sentences.
    CoreferenceSentence *sentence = document->GetSentence(i);
    sentence_writer_.Write(sentence);
  }
  os_ << "#end document" << std::endl;
}

void CoreferenceWriter::WriteFormatted(Pipe * pipe, Instance *instance) {}

void CoreferenceSentenceWriter::Write(Instance *instance) {
  CoreferenceSentence *sentence = static_cast<CoreferenceSentence*>(instance);
  std::ofstream *os = (external_os_) ? external_os_ : &os_;

  std::vector<std::string> span_lines;
  ConstructTextFromCoreferenceSpans(sentence->size(),
                                    sentence->GetCoreferenceSpans(),
                                    &span_lines);

  for (int i = 1; i < sentence->size(); ++i) {
    *os << sentence->GetName() << "\t";
    *os << i - 1 << "\t";
    *os << sentence->GetForm(i) << "\t";
    //os_ << sentence->GetLemma(i) << "\t";
    // No distiction between pos and cpos.
    //os_ << sentence->GetCoarsePosTag(i) << "\t";
    *os << sentence->GetPosTag(i) << "\t";
    //os_ << sentence->GetHead(i) << "\t";
    //os_ << sentence->GetDependencyRelation(i) << endl;

    // Parse span (e.g. "(NP (NN *")); empty for now.
    *os << "_" << "\t"; // Change this later

    // Semantic role labeling information; empty for now.
    *os << "_" << "\t"; // Change this later
    *os << "_" << "\t"; // Change this later

    // Word sense; empty for now.
    *os << "_" << "\t"; // Change this later

    // Author info; empty for now.
    *os << "_" << "\t"; // Change this later

    // Entity info; empty for now.
    *os << "_" << "\t"; // Change this later

    // Coreference info.
    *os << span_lines[i] << std::endl;
  }
  *os << std::endl;
}

void CoreferenceSentenceWriter::WriteFormatted(Pipe * pipe, Instance *instance) {}

#if 0
void CoreferenceSentenceReader::ConstructSpansFromText(
  const std::vector<std::string> &span_lines,
  std::vector<NamedSpan*> *spans) {
  char left_bracket = '(';
  char right_bracket = ')';
  std::string characters_to_ignore = "*-";
  std::string name = "";
  std::stack<std::string> span_names_stack;
  std::stack<int> span_start_stack;

  for (int i = 0; i < span_lines.size(); ++i) {
    std::string line = span_lines[i];
    //LOG(INFO) << line;
    for (int j = 0; j < line.length(); ++j) {
      char ch = line[j];
      if (ch == left_bracket) {
        name = "";
        span_start_stack.push(i);
      } else if (ch == right_bracket) {
        name = span_names_stack.top();
        int start_position = span_start_stack.top();
        int end_position = i;
        span_names_stack.pop();
        span_start_stack.pop();
        NamedSpan *span = new NamedSpan(start_position, end_position, name);
        spans->push_back(span);
      } else if (characters_to_ignore.find(ch) != std::string::npos) {
        continue;
      } else {
        name += ch;
        if (j + 1 >= line.length() ||
            line[j + 1] == left_bracket || line[j + 1] == right_bracket ||
            characters_to_ignore.find(line[j + 1]) != std::string::npos) {
          span_names_stack.push(name);
        }
      }
    }
  }

  CHECK_EQ(span_names_stack.size(), 0);
  CHECK_EQ(span_start_stack.size(), 0);
}
#endif

void CoreferenceSentenceWriter::ConstructTextFromCoreferenceSpans(
  int length,
  const std::vector<NamedSpan*> &spans,
  std::vector<std::string> *span_lines) {
  char left_bracket = '(';
  char right_bracket = ')';

  //std::cout << "New sentence:" << std::endl;
  // Length is number of words including the root symbol.
  span_lines->assign(length, "");
  for (int j = 0; j < spans.size(); ++j) {
#if 0
    std::cout << spans[j]->name() << " (" << spans[j]->start() << ", "
      << spans[j]->end() << ")" << std::endl;
#endif
    if (spans[j]->start() == spans[j]->end()) {
      // Single-word span.
      if ((*span_lines)[spans[j]->start()] != "") {
        (*span_lines)[spans[j]->start()] += "|";
      }
      (*span_lines)[spans[j]->start()] += left_bracket + spans[j]->name() +
        right_bracket;
    } else {
      // Multi-word span.
      std::string line = (*span_lines)[spans[j]->start()];
      if (line != "") line = "|" + line;
      (*span_lines)[spans[j]->start()] = left_bracket + spans[j]->name() + line;
      line = (*span_lines)[spans[j]->end()];
      if (line != "") line += "|";
      (*span_lines)[spans[j]->end()] = line + spans[j]->name() + right_bracket;
    }
  }

  for (int i = 0; i < span_lines->size(); ++i) {
    if ((*span_lines)[i] == "") (*span_lines)[i] = "_";
  }
}
