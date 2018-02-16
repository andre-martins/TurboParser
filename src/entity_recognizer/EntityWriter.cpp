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

#include "EntityPipe.h"
#include "EntityWriter.h"
#include "EntityInstance.h"
#include "EntityInstanceNumeric.h"
#include "EntityOptions.h"
#include <iostream>
#include <sstream>

void EntityWriter::Write(Instance *instance) {
  EntityInstance *entity_instance =
    static_cast<EntityInstance*>(instance);

  // Always write in BIO format.
  std::vector<EntitySpan*> spans;
  std::vector<std::string> tags;
  entity_instance->CreateSpansFromTags(entity_instance->tags(), &spans);
  entity_instance->CreateTagsFromSpans(entity_instance->tags().size(),
                                       spans,
                                       EntityTaggingSchemes::BIO,
                                       &tags);
  entity_instance->DeleteSpans(&spans);

  for (int i = 0; i < entity_instance->size(); ++i) {
    os_ << entity_instance->GetForm(i) << "\t";
    os_ << entity_instance->GetPosTag(i) << "\t";
    os_ << tags[i] << endl;
  }
  os_ << endl;
}

void EntityWriter::WriteFormatted(Pipe * pipe, Instance *instance) {
  if ((static_cast<EntityPipe*>(pipe))->GetEntityOptions()->expose_node_edge_viterbi_scores()) {
    EntityInstanceNumeric *entity_instance =
      static_cast<EntityInstanceNumeric*>(instance);

    EntityPipe * entity_pipe =
      static_cast<EntityPipe*>(pipe);

    for (int i = 0; i < entity_instance->size(); ++i) {
      for (int j = 0; j < entity_instance->node_scores_[i].GetNumStates(); ++j) {
        if (j > 0)
          os_formatted_ << "\t";
        os_formatted_ <<
          entity_pipe->GetEntityDictionary()->GetTagName(entity_instance->node_scores_[i].GetState(j)) << ":" << entity_instance->node_scores_[i].GetScore(j);
      }

      os_formatted_ << "\t";
      if (i < entity_instance->size() - 1) {
        for (int j = 0; j < entity_instance->edge_scores_[i].GetNumCurrentStates(); ++j) {
          int tag_id = entity_instance->node_scores_[i + 1].GetState(j);
          if (j > 0)
            os_formatted_ << "\t";
          for (int k = 0; k < entity_instance->edge_scores_[i].GetNumPreviousStates(j); ++k) {
            if (k > 0)
              os_formatted_ << "\t";

            int tag_left_id = entity_instance->node_scores_[i].GetState(entity_instance->edge_scores_[i].GetAllPreviousStateScores(j)[k].first);
            os_formatted_ <<
              entity_pipe->GetEntityDictionary()->GetTagName(tag_left_id) << "->" <<
              entity_pipe->GetEntityDictionary()->GetTagName(tag_id) << ":"
              << entity_instance->edge_scores_[i].GetAllPreviousStateScores(j)[k].second;
          }
        }
      }
      os_formatted_ << endl;
    }
    os_formatted_ << endl;
  }
}

