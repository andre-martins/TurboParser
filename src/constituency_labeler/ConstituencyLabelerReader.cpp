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

#include "ConstituencyLabelerReader.h"
#include "ConstituencyLabelerInstance.h"
#include "Utils.h"
#include <iostream>
#include <sstream>

Instance *ConstituencyLabelerReader::GetNext() {
  // Fill all fields for the entire sentence.
  ConstituencyLabelerInstance *instance = NULL;
  std::string line;
  if (is_.is_open() && !is_.eof()) {
    getline(is_, line);
    if (line != "") {
      ParseTree tree;
      std::vector<std::string> words;
      std::vector<std::string> tags;
      std::vector<std::string> lemmas;
      std::vector<std::vector<std::string> > feats;
      tree.LoadFromString(line);
      tree.ExtractWordsAndTags(&words, &tags);

      // Extract lemma and morpho-syntactic information, if available.
      // The format is "TAG##lem=LEMMA|MORPH##".
      // Example: "DET##lem=hogeita_hamabost|AZP=DET_DZH|KAS=ZERO##"
      lemmas.resize(tags.size());
      feats.resize(tags.size());
      std::string start_info_marker = "##";
      std::string end_info_marker = "##";
      for (int i = 0; i < tags.size(); ++i) {
        std::string tag = tags[i];
        ExtractLemmasAndMorphFeatsFromTag(tag, &tags[i], &lemmas[i],
                                          &feats[i]);

#if 0
        std::string all_morph_feats = "";
        StringJoin(feats[i], '+', &all_morph_feats);
        LOG(INFO) << "Word=" << words[i] << " "
                  << "Tag=" << tags[i] << " "
                  << "Lemma=" << lemmas[i] << " "
                  << "Morph=" << all_morph_feats;
#endif
      }

      // Clean lemma/morpho-syntactic information from the original nodes.
      for (int i = 0; i < tree.non_terminals().size(); ++i) {
        if (!tree.non_terminals()[i]->IsPreTerminal()) continue;
        std::string full_label = tree.non_terminals()[i]->label();
        std::string tag, lemma;
        std::vector<std::string> morph_feats;
        ExtractLemmasAndMorphFeatsFromTag(full_label, &tag, &lemma,
                                          &morph_feats);
        tree.non_terminals()[i]->set_label(tag);
      }

      // Collapse unary spines into a single node with labels split by "|".
      tree.CollapseSingletonSpines(false, true);

      //std::cout << line << std::endl;

      const std::vector<ParseTreeNode*> &terminals = tree.terminals();
      const std::vector<ParseTreeNode*> &non_terminals = tree.non_terminals();
      std::vector<std::string> constituent_labels;
      for (int i = 0; i < non_terminals.size(); ++i) {
        std::string full_label = non_terminals[i]->label();
        std::vector<std::string> labels;
        std::string delim = "";
        delim += kParseTreeLabelSeparator;
        StringSplit(full_label, delim, &labels, true);
        if (labels.size() > 1) {
          non_terminals[i]->set_label(labels.back());
          labels.pop_back();
          std::string label;
          StringJoin(labels, kParseTreeLabelSeparator, &label);
          constituent_labels.push_back(label);
        } else {
          constituent_labels.push_back(""); // NULL label.
        }
      }
      instance = new ConstituencyLabelerInstance;
      instance->Initialize(words, lemmas, tags, feats, tree,
                           constituent_labels);

      CHECK_EQ(terminals.size(), instance->size());

#if 0
      std::string info;
      instance->GetParseTree().SaveToString(&info);
      std::cout << info << std::endl;
      for (int i = 0; i < instance->size(); ++i) {
        std::cout << instance->GetForm(i) << "\t"
                  << instance->GetTag(i) << std::endl;
      }
      CHECK_EQ(terminals.size(), instance->size());
      for (int i = 0; i < terminals.size(); ++i) {
        std::cout << terminals[i]->label() << "("
                  << terminals[i]->start() << ","
                  << terminals[i]->end() << ")"
                  << " fathered by "
                  << terminals[i]->parent()->label() << "("
                  << terminals[i]->parent()->start() << ","
                  << terminals[i]->parent()->end() << ")"
                  << endl;
      }
      for (int i = 0; i < non_terminals.size(); ++i) {
        CHECK(non_terminals[i]);
        std::cout << non_terminals[i]->label() << "("
                  << non_terminals[i]->start() << ","
                  << non_terminals[i]->end() << ")";
        if (non_terminals[i]->parent() != NULL) {
          std::cout << " fathered by "
                    << non_terminals[i]->parent()->label() << "("
                    << non_terminals[i]->parent()->start() << ","
                    << non_terminals[i]->parent()->end() << ")";
        }
        std::cout << " -> " << instance->GetConstituentLabels()[i] << endl;
      }
#endif

    }
  }
  return static_cast<Instance*>(instance);
}

void ConstituencyLabelerReader::ExtractLemmasAndMorphFeatsFromTag(
    const std::string &original_tag,
    std::string *tag,
    std::string *lemma,
    std::vector<std::string> *morph_feats) {

  std::string start_info_marker = "##";
  std::string end_info_marker = "##";

  *tag = original_tag;
  *lemma = "";
  morph_feats->clear();

  std::string info = "_";
  std::size_t start_info = original_tag.find(start_info_marker);
  std::size_t end_info = original_tag.rfind(end_info_marker);
  if (start_info != original_tag.npos && end_info != start_info) {
    *tag = original_tag.substr(0, start_info);
    start_info += start_info_marker.length();
    info = original_tag.substr(start_info, end_info - start_info);
  }

  std::string feat_seq = info;
  if (0 == feat_seq.compare("_")) {
    *lemma = "_";
    morph_feats->clear();
  } else {
    StringSplit(feat_seq, "|", morph_feats, true);
    for (int j = 0; j < morph_feats->size(); ++j) {
      std::string lemma_prefix = "lem=";
      if ((*morph_feats)[j].compare(0, lemma_prefix.length(),
                                    lemma_prefix) == 0) {
        *lemma = (*morph_feats)[j].substr(lemma_prefix.length());
        morph_feats->erase(morph_feats->begin() + j);
        break;
      }
    }
  }
}
