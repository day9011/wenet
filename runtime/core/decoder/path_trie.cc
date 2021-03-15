// Copyright 2021 Mobvoi Inc. All Rights Reserved.
// Author: zhendong.peng@mobvoi.com (Zhendong Peng)

#include "decoder/path_trie.h"

namespace wenet {

PathTrie::PathTrie(int id, float prob, int time_step, PathTrie* parent) {
  prob_b_prev_ = -kFloatMax;
  prob_nb_prev_ = -kFloatMax;
  score_ = -kFloatMax;
  prob_b_cur_ = -kFloatMax;
  prob_nb_cur_ = -kFloatMax;

  exists_ = true;
  id_ = id;
  prob_ = prob;
  time_step_ = time_step;
  parent_ = parent;
  children_.clear();
}

PathTrie::~PathTrie() {
  for (auto child : children_) {
    delete child;
  }
}

bool PathTrie::PrefixCompare(const PathTrie* x, const PathTrie* y) {
  if (x->score_ == y->score_) {
    return x->id_ < y->id_;
  }
  return x->score_ > y->score_;
}

void PathTrie::Append(int id, float prob, int time_step) {
  // Case 0: * + ε => *
  if (id == blank_) {
    prob_b_cur_ = LogAdd(prob_b_cur_, score_ + prob);
    return;
  }

  auto new_prefix = GetPathTrie(id, prob, time_step);
  if (id == id_) {
    // Case 1: *a + a => *a
    prob_nb_cur_ = LogAdd(prob_nb_cur_, prob_nb_prev_ + prob);
    // Case 2: *aε + a => *aa
    new_prefix->prob_nb_cur_ =
            LogAdd(new_prefix->prob_nb_cur_, prob_b_prev_ + prob);
  } else {
    // Case 3: *a + b => *ab
    new_prefix->prob_nb_cur_ = LogAdd(new_prefix->prob_nb_cur_, score_ + prob);
  }
}

PathTrie* PathTrie::GetPathTrie(int id, float prob, int time_step) {
  // Prefix exists
  for (auto& child : children_) {
    if (child->id_ == id) {
      if (child->prob_ < prob) {
        child->prob_ = prob;
        child->time_step_ = time_step;
      }
      return child;
    }
  }
  // Prefix not exists
  auto* new_path = new PathTrie(id, prob, time_step, this);
  children_.emplace_back(new_path);
  return new_path;
}

PathTrie* PathTrie::GetPathVec(std::vector<int>* output,
                               std::vector<int>* time_steps) {
  if (id_ == root_) {
    std::reverse(output->begin(), output->end());
    std::reverse(time_steps->begin(), time_steps->end());
    return this;
  }
  output->emplace_back(id_);
  time_steps->emplace_back(time_step_);
  return parent_->GetPathVec(output, time_steps);
}

void PathTrie::UpdatePrefixes() {
  if (exists_) {
    prob_b_prev_ = prob_b_cur_;
    prob_nb_prev_ = prob_nb_cur_;
    score_ = LogAdd(prob_b_prev_, prob_nb_prev_);
    prob_b_cur_ = -kFloatMax;
    prob_nb_cur_ = -kFloatMax;
  }
  for (auto child : children_) {
    child->UpdatePrefixes();
  }
}

void PathTrie::GetPrefixes(std::vector<PathTrie*>* output) {
  if (exists_) {
    output->emplace_back(this);
  }
  for (auto child : children_) {
    child->GetPrefixes(output);
  }
}

void PathTrie::Remove() {
  exists_ = false;
  // Current path has no children, means the path is not end with ε
  if (children_.empty()) {
    auto& children = parent_->children_;
    for (auto child = children.begin(); child != children.end(); child++) {
      if ((*child)->id_ == id_) {
        children.erase(child);
        break;
      }
    }
    // Current deleted node has no brothers
    if (children.empty() && !parent_->exists_) {
      parent_->Remove();
    }
    delete this;
  }
}

}  // namespace wenet
