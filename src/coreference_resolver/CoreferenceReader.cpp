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

#include "CoreferenceReader.h"
#include "CoreferenceOptions.h"
#include "Utils.h"
#include <stack>
#include <iostream>
#include <sstream>

Instance *CoreferenceReader::GetNext() {
  int num_sentences = 0;
  std::string line;
  std::string name = "";
  int part_number = 0;
  bool found_begin = false;
  bool found_end = false;
  if (is_.is_open()) {
    while (!is_.eof()) {
      getline(is_, line);
      // E.g. "#begin document (nw/wsj/02/wsj_0242); part 000"
      if (0 == line.substr(0, 6).compare("#begin")) {
        name = line;
        part_number = 0;
        // Extract name.
        size_t start = line.find("(");
        size_t end = line.find(")");
        CHECK_NE(start, std::string::npos);
        CHECK_NE(end, std::string::npos);
        CHECK_LT(start, end);
        CHECK_LT(end + 1, line.size());
        CHECK_EQ(line[end + 1], ';');
        name = line.substr(start + 1, end - start - 1); // Document id.
        // Extract part number.
        start = line.find("part ");
        CHECK_NE(start, std::string::npos);
        start += 5;
        end = line.find_first_not_of("0123456789", start);
        if (end == std::string::npos) end = line.length();
        CHECK_LT(start, end);
        std::string part_name = line.substr(start, end - start);
        std::stringstream ss(part_name);
        ss >> part_number; // Document part number.

        //LOG(INFO) << "Document: " << name << " Part: " << part_number;
        CHECK(!found_begin);
        CHECK(!found_end);
        found_begin = true;
      } else if (0 == line.substr(0, 4).compare("#end")) {
        // End of document.
        CHECK(found_begin);
        CHECK(!found_end);
        found_end = true;
        break;
      } else if (line.length() == 0) {
        // End of sentence; update counter.
        if (found_begin) ++num_sentences;
      }
    }
  }

  // Now read all the sentences in this document.
  std::vector<CoreferenceSentence*> sentences;
  for (int i = 0; i < num_sentences; ++i) {
    // Start reading sentences.
    //LOG(INFO) << "Reading sentence #" << i << "...";
    CoreferenceSentence *sentence =
      static_cast<CoreferenceSentence*>(sentence_reader_.GetNext());
    CHECK(sentence);
    sentences.push_back(sentence);
  }

  // Create document instance.
  if (num_sentences == 0 && is_.is_open() && is_.eof()) return NULL;

  //LOG(INFO) << "Found " << num_sentences << " sentences in document " << name << ".";

  CoreferenceDocument *instance = new CoreferenceDocument;
  instance->Initialize(name, part_number, sentences);

  return static_cast<Instance*>(instance);
}

Instance *CoreferenceSentenceReader::GetNext() {
  // Fill all fields for the entire sentence.
  std::string name = "";
  //int part_number = 0;
  std::vector<std::vector<std::string> > sentence_fields;
  std::string line;

  if (is_.is_open()) {
    while (!is_.eof()) {
      getline(is_, line);
      if (line.length() <= 0) break;
      if (0 == line.substr(0, 1).compare("#")) {
        continue;
      }
      //LOG(INFO) << line;
      std::vector<std::string> fields;
      StringSplit(line, "\t ", &fields, true);
      sentence_fields.push_back(fields);
    }
  }

  bool read_next_sentence = false;
  if (!is_.eof()) read_next_sentence = true;

  // Sentence length.
  int length = (int)sentence_fields.size();

  //LOG(INFO) << "Sentence has length " << length;

  // Convert to array of forms, lemmas, etc.
  // Note: the first token is the root symbol.
  std::vector<std::string> forms(length + 1);
  std::vector<std::string> lemmas(length + 1);
  std::vector<std::string> cpos(length + 1);
  std::vector<std::string> pos(length + 1);
  std::vector<std::vector<std::string> > feats(length + 1);
  std::vector<std::string> deprels(length + 1);
  std::vector<int> heads(length + 1);
  std::vector<std::string> predicate_names; // Names of predicates (e.g. "take.01").
  std::vector<int> predicate_indices; // Positions of each predicate in the sentence.
  std::vector<std::vector<std::string> > argument_roles; // Semantic roles.
  std::vector<std::vector<int> > argument_indices; // Positions of each argument.
  std::vector<std::string> parse_info(length + 1);
  std::vector<std::string> author_info(length + 1);
  std::vector<std::string> entity_info(length + 1);
  std::vector<std::string> coreference_info(length + 1);

  forms[0] = "_root_";
  lemmas[0] = "_root_";
  cpos[0] = "_root_";
  pos[0] = "_root_";
  deprels[0] = "_root_";
  heads[0] = -1;
  feats[0] = std::vector<std::string>(1, "_root_");
  parse_info[0] = "*";
  author_info[0] = "__";
  entity_info[0] = "*";
  coreference_info[0] = "-";

  int num_predicates = 0;
  for (int i = 0; i < length; i++) {
    const vector<string> &info = sentence_fields[i];

    int offset = 0;
    if (i == 0) {
      name = info[offset] + "\t" + info[offset + 1]; // Document name and part.
    }
    offset += 3;

    // Use splitted forms.
    forms[i + 1] = info[offset];
    lemmas[i + 1] = "_";
    ++offset;
    cpos[i + 1] = info[offset];
    pos[i + 1] = info[offset]; // No distiction between pos and cpos.
    ++offset;
    parse_info[i + 1] = info[offset]; // Parse span (e.g. "(NP (NN *")
    ++offset;

    // No morpho-syntactic information.
    feats[i + 1].clear();

    // Dependency syntactic information.
    if (false) { //!semantic_options->use_dependency_syntactic_features()) {
      heads[i + 1] = 0;
      deprels[i + 1] = "NULL";
    } else {
      std::stringstream ss(info[offset]);
      ++offset;
      ss >> heads[i + 1];
      //++heads[i+1]; // Note: heads start at -1 here!
      deprels[i + 1] = info[offset];
      ++offset;

      if (heads[i + 1] < 0 || heads[i + 1] > length) {
        LOG(INFO) << "Invalid value of head (" << heads[i + 1]
          << ") not in range [0.." << length
          << "] - attaching to the root.";
        heads[i + 1] = 0;
      }
    }

    // Semantic role labeling information.
    bool is_predicate = false;
    std::string predicate_name = info[offset];
    ++offset;
    std::string predicate_sense = info[offset];
    ++offset;
    // Added the comparison below due to the CONLL 2012 data.
    if (0 != predicate_sense.compare("_") &&
        0 != predicate_sense.compare("-")) {
      predicate_name += "." + predicate_sense; // Predicate lemma+sense in PropBank.
      is_predicate = true;
    }
    //    if (0 != predicate_name.compare("_") &&
    //    0 != predicate_name.compare("-")) {
    //}

    std::string word_sense = info[offset];
    ++offset;
    author_info[i + 1] = info[offset];
    ++offset;
    entity_info[i + 1] = info[offset];
    ++offset;

    if (i == 0) {
      // Allocate space for predicates.
      num_predicates = (int)info.size() - 1 - offset;
      //LOG(INFO) << num_predicates;
      // Top nodes will be considered arguments of a special root node.
      if (use_top_nodes_) ++num_predicates;
      //LOG(INFO) << num_predicates;
      predicate_names.resize(num_predicates);
      predicate_indices.resize(num_predicates);
      argument_roles.resize(num_predicates);
      argument_indices.resize(num_predicates);
      num_predicates = 0;
      if (use_top_nodes_) {
        predicate_names[num_predicates] = "__ROOT__";
        predicate_indices[num_predicates] = 0;
        ++num_predicates;
      }
    }

    if (is_predicate) {
      //LOG(INFO) << predicate_name;
      CHECK_LT(num_predicates, predicate_names.size());
      predicate_names[num_predicates] = predicate_name;
      predicate_indices[num_predicates] = i + 1;
      ++num_predicates;
    }

    // Note: this is assuming arguments are encoded as dependents.
    // However, in some datasets (e.g. Ontonotes) arguments are encoded as
    // spans.
    for (int j = offset; j < info.size() - 1; ++j) {
      string argument_role = info[j];
      bool is_argument = false;
      if (0 != argument_role.compare("_")) is_argument = true;
      if (is_argument) {
        int k = j - offset;
        if (use_top_nodes_) ++k;
        argument_roles[k].push_back(argument_role);
        argument_indices[k].push_back(i + 1);
      }
    }
    offset = (int)info.size() - 1;

    // Add coreference information.
    coreference_info[i + 1] = info[offset];
    ++offset;
  }

  CHECK_EQ(num_predicates, predicate_names.size());

  // TODO: Create spans...
  std::vector<EntitySpan*> entity_spans;
  std::vector<NamedSpan*> constituent_spans;
  std::vector<NamedSpan*> coreference_spans;

  ConstructSpansFromText(entity_info, &entity_spans);
  ConstructSpansFromText(parse_info, &constituent_spans);
  ConstructCoreferenceSpansFromText(coreference_info, &coreference_spans);

#if 0
  for (int k = 0; k < entity_spans.size(); ++k) {
    LOG(INFO) << "Entity " << entity_spans[k]->name()
      << "(" << entity_spans[k]->start()
      << "," << entity_spans[k]->end()
      << ")";
  }
  for (int k = 0; k < constituent_spans.size(); ++k) {
    LOG(INFO) << "Constituent " << constituent_spans[k]->name()
      << "(" << constituent_spans[k]->start()
      << "," << constituent_spans[k]->end()
      << ")";
  }
  for (int k = 0; k < coreference_spans.size(); ++k) {
    LOG(INFO) << "Mention " << coreference_spans[k]->name()
      << " (" << coreference_spans[k]->start()
      << "," << coreference_spans[k]->end()
      << ")";
  }
#endif

  CoreferenceSentence *instance = NULL;
  if (read_next_sentence && length >= 0) {
    instance = new CoreferenceSentence;
    instance->Initialize(name, forms, lemmas, cpos, pos, feats, deprels, heads,
                         predicate_names, predicate_indices, argument_roles,
                         argument_indices, author_info, entity_spans,
                         constituent_spans, coreference_spans);
  }

  return static_cast<Instance*>(instance);
}

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

void CoreferenceSentenceReader::ConstructCoreferenceSpansFromText(
  const std::vector<std::string> &span_lines,
  std::vector<NamedSpan*> *spans) {
  char left_bracket = '(';
  char right_bracket = ')';
  std::string characters_to_ignore = "*-";
  std::string name = "";

  for (int i = 0; i < span_lines.size(); ++i) {
    std::string line = span_lines[i];
    std::vector<std::string> fields;
    StringSplit(line, "|", &fields, true);
    //LOG(INFO) << "Span " << i << ": " << line;
    for (int j = 0; j < fields.size(); ++j) {
      std::string field = fields[j];
      CHECK_GE(field.length(), 1);
      char first_ch = field[0];
      char last_ch = field[field.length() - 1];
      if (first_ch == left_bracket && last_ch == right_bracket) {
        int start_position = i;
        int end_position = i;
        name = field.substr(1, field.length() - 2);
        NamedSpan *span = new NamedSpan(start_position, end_position, name);
        spans->push_back(span);
      } else if (first_ch == left_bracket) {
        int start_position = i;
        int end_position = -1;
        name = field.substr(1, field.length() - 1);
        NamedSpan *span = new NamedSpan(start_position, end_position, name);
        spans->push_back(span);
      } else if (last_ch == right_bracket) {
        name = field.substr(0, field.length() - 1);
        NamedSpan *selected_span = NULL;
        for (int k = spans->size() - 1; k >= 0; --k) {
          if ((*spans)[k]->name() == name && (*spans)[k]->end() < 0) {
            CHECK(!selected_span);
            selected_span = (*spans)[k];
            break;
          }
        }
        CHECK(selected_span);
        selected_span->set_end(i);
      }
    }
  }

  for (int k = 0; k < spans->size(); ++k) {
    CHECK_GE((*spans)[k]->end(), 0);
  }
}
