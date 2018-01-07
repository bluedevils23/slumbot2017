// Meant for use with solve_all_endgames4.  solve_all_endgames4 is limited
// in terms of what types of endgame solving it does.  There is no nested
// endgame solving, and endgames are always solved at street-initial nodes.
//
// We also write out endgames in a different directory format.
//
// We use routines from endgame_utils.cpp here.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "betting_abstraction.h"
#include "betting_abstraction_params.h"
#include "betting_tree.h"
#include "board_tree.h"
#include "buckets.h"
#include "canonical_cards.h"
#include "card_abstraction.h"
#include "card_abstraction_params.h"
#include "cfr_config.h"
#include "cfr_params.h"
#include "cfr_utils.h"
#include "cfr_values.h"
#include "eg_cfr.h" // ResolvingMethod
#include "endgame_utils.h" // ReadEndgame()
#include "files.h"
#include "game.h"
#include "game_params.h"
#include "io.h"
#include "params.h"
#include "split.h"

class Assembler {
public:
  Assembler(BettingTree *base_betting_tree, BettingTree *endgame_betting_tree,
	    unsigned int solve_street, unsigned int base_it,
	    unsigned int endgame_it, const CardAbstraction &base_ca,
	    const CardAbstraction &endgame_ca,
	    const CardAbstraction &merged_ca,
	    const BettingAbstraction &base_ba,
	    const BettingAbstraction &endgame_ba, const CFRConfig &base_cc,
	    const CFRConfig &endgame_cc, const CFRConfig &merged_cc,
	    ResolvingMethod method);
  Assembler(void);
  ~Assembler(void);
  void Go(void);
private:
  void WalkTrunk(Node *base_node, Node *endgame_node,
		 const string &action_sequence, unsigned int last_st);

  bool asymmetric_;
  BettingTree *base_betting_tree_;
  BettingTree *endgame_betting_tree_;
  unsigned int solve_street_;
  unsigned int base_it_;
  unsigned int endgame_it_;
  const CardAbstraction &base_ca_;
  const CardAbstraction &endgame_ca_;
  const CardAbstraction &merged_ca_;
  const BettingAbstraction &base_ba_;
  const BettingAbstraction &endgame_ba_;
  const CFRConfig &base_cc_;
  const CFRConfig &endgame_cc_;
  const CFRConfig &merged_cc_;
  ResolvingMethod method_;
  unique_ptr<CFRValues> base_sumprobs_;
  unique_ptr<CFRValues> merged_sumprobs_;
  unique_ptr<Buckets> endgame_buckets_;
};

Assembler::Assembler(BettingTree *base_betting_tree,
		     BettingTree *endgame_betting_tree,
		     unsigned int solve_street, unsigned int base_it,
		     unsigned int endgame_it,
		     const CardAbstraction &base_ca,
		     const CardAbstraction &endgame_ca,
		     const CardAbstraction &merged_ca,
		     const BettingAbstraction &base_ba,
		     const BettingAbstraction &endgame_ba,
		     const CFRConfig &base_cc,
		     const CFRConfig &endgame_cc,
		     const CFRConfig &merged_cc,
		     ResolvingMethod method) :
  base_ca_(base_ca), endgame_ca_(endgame_ca), merged_ca_(merged_ca),
  base_ba_(base_ba), endgame_ba_(endgame_ba), base_cc_(base_cc),
  endgame_cc_(endgame_cc), merged_cc_(merged_cc) {
  asymmetric_ = false;
  base_betting_tree_ = base_betting_tree;
  endgame_betting_tree_ = endgame_betting_tree;
  solve_street_ = solve_street;
  base_it_ = base_it;
  endgame_it_ = endgame_it;
  method_ = method;

  DeleteOldFiles(merged_ca_, endgame_ba_, merged_cc_, endgame_it_);

  endgame_buckets_.reset(new Buckets(endgame_ca_, true));
  
  unsigned int max_street = Game::MaxStreet();
  unique_ptr<bool []> base_streets(new bool[max_street + 1]);
  for (unsigned int st = 0; st <= max_street; ++st) {
    base_streets[st] = st < solve_street_;
  }
  Buckets base_buckets(base_ca_, true);

  unique_ptr<bool []> base_compressed_streets(new bool[max_street + 1]);
  for (unsigned int st = 0; st <= max_street; ++st) {
    base_compressed_streets[st] = false;
  }
  const vector<unsigned int> &bcsv = base_cc_.CompressedStreets();
  unsigned int num_bcsv = bcsv.size();
  for (unsigned int i = 0; i < num_bcsv; ++i) {
    unsigned int st = bcsv[i];
    base_compressed_streets[st] = true;
  }

  CFRValues base_sumprobs(nullptr, true, base_streets.get(),
			  base_betting_tree_, 0, 0, base_ca_,
			  base_buckets.NumBuckets(),
			  base_compressed_streets.get());
  
  char read_dir[500], write_dir[500];
  sprintf(read_dir, "%s/%s.%u.%s.%u.%u.%u.%s.%s", Files::OldCFRBase(),
	  Game::GameName().c_str(), Game::NumPlayers(),
	  base_ca.CardAbstractionName().c_str(), Game::NumRanks(),
	  Game::NumSuits(), Game::MaxStreet(),
	  base_ba_.BettingAbstractionName().c_str(),
	  base_cc_.CFRConfigName().c_str());
  base_sumprobs.Read(read_dir, base_it, base_betting_tree_->Root(), "x",
		     kMaxUInt);
  sprintf(write_dir, "%s/%s.%u.%s.%u.%u.%u.%s.%s", Files::NewCFRBase(),
	  Game::GameName().c_str(), Game::NumPlayers(),
	  merged_ca_.CardAbstractionName().c_str(), Game::NumRanks(),
	  Game::NumSuits(), Game::MaxStreet(),
	  endgame_ba_.BettingAbstractionName().c_str(),
	  merged_cc_.CFRConfigName().c_str());
  base_sumprobs.Write(write_dir, endgame_it_, base_betting_tree_->Root(), "x",
		      kMaxUInt);

  unique_ptr<bool []> endgame_streets(new bool[max_street + 1]);
  for (unsigned int st = 0; st <= max_street; ++st) {
    endgame_streets[st] = st >= solve_street_;
  }
  Buckets merged_buckets(merged_ca_, true);

  unique_ptr<bool []> merged_compressed_streets(new bool[max_street + 1]);
  for (unsigned int st = 0; st <= max_street; ++st) {
    merged_compressed_streets[st] = false;
  }
  const vector<unsigned int> &mcsv = merged_cc_.CompressedStreets();
  unsigned int num_mcsv = mcsv.size();
  for (unsigned int i = 0; i < num_mcsv; ++i) {
    unsigned int st = mcsv[i];
    merged_compressed_streets[st] = true;
  }

  merged_sumprobs_.reset(new CFRValues(nullptr, true, endgame_streets.get(),
				       endgame_betting_tree_, 0, 0,
				       merged_ca_, merged_buckets.NumBuckets(),
				       merged_compressed_streets.get()));
}

Assembler::~Assembler(void) {
}

// When we get to the target street, read the entire base strategy for
// this subtree into merged sumprobs.  Then go through and override parts
// with the endgame strategy.
void Assembler::WalkTrunk(Node *base_node, Node *endgame_node,
			  const string &action_sequence,
			  unsigned int last_st) {
  if (base_node->Terminal()) return;
  unsigned int st = base_node->Street();
  if (st == solve_street_) {
    unsigned int num_boards = BoardTree::NumBoards(st);
    fprintf(stderr, "%s\n", action_sequence.c_str());
    BettingTree *subtree = BettingTree::BuildSubtree(endgame_node);
    unsigned int max_street = Game::MaxStreet();
    bool *endgame_streets = new bool[max_street + 1];
    for (unsigned int st = 0; st <= max_street; ++st) {
      endgame_streets[st] = st >= solve_street_;
    }
    bool *endgame_compressed_streets = new bool[max_street + 1];
    for (unsigned int st = 0; st <= max_street; ++st) {
      endgame_compressed_streets[st] = false;
    }
    const vector<unsigned int> &ecsv = endgame_cc_.CompressedStreets();
    unsigned int num_ecsv = ecsv.size();
    for (unsigned int i = 0; i < num_ecsv; ++i) {
      unsigned int st = ecsv[i];
      endgame_compressed_streets[st] = true;
    }
    for (unsigned int gbd = 0; gbd < num_boards; ++gbd) {
      unique_ptr<CFRValues> endgame_sumprobs;
      endgame_sumprobs.reset(ReadEndgame(action_sequence, subtree, gbd,
					 base_ca_, endgame_ca_, base_ba_,
					 endgame_ba_, base_cc_, endgame_cc_,
					 *endgame_buckets_.get(), method_, st,
					 gbd, kMaxUInt));
      merged_sumprobs_->MergeInto(*endgame_sumprobs.get(), gbd, endgame_node,
				  subtree->Root(), *endgame_buckets_,
				  Game::MaxStreet());
    }
    
    delete subtree;
    delete [] endgame_streets;
    delete [] endgame_compressed_streets;
    
    return;
  }
  unsigned int num_succs = base_node->NumSuccs();
  for (unsigned int s = 0; s < num_succs; ++s) {
    string action = base_node->ActionName(s);
    WalkTrunk(base_node->IthSucc(s), endgame_node->IthSucc(s),
	      action_sequence + action, st);
  }
}

void Assembler::Go(void) {
  WalkTrunk(base_betting_tree_->Root(), endgame_betting_tree_->Root(), "x",
	    base_betting_tree_->Root()->Street());
  char dir[500];
  sprintf(dir, "%s/%s.%u.%s.%u.%u.%u.%s.%s", Files::NewCFRBase(),
	  Game::GameName().c_str(), Game::NumPlayers(),
	  merged_ca_.CardAbstractionName().c_str(), Game::NumRanks(),
	  Game::NumSuits(), Game::MaxStreet(),
	  endgame_ba_.BettingAbstractionName().c_str(),
	  merged_cc_.CFRConfigName().c_str());
  merged_sumprobs_->Write(dir, endgame_it_, endgame_betting_tree_->Root(),
			  "x", kMaxUInt);
}

static void Usage(const char *prog_name) {
  fprintf(stderr, "USAGE: %s <game params> <base card params> "
	  "<endgame card params> <merged card params> <base betting params> "
	  "<endgame betting params> <base CFR params> <endgame CFR params> "
	  "<merged CFR params> <solve street> <base it> <endgame it> "
	  "<method>\n", prog_name);
  exit(-1);
}

int main(int argc, char *argv[]) {
  if (argc != 14) Usage(argv[0]);
  Files::Init();
  unique_ptr<Params> game_params = CreateGameParams();
  game_params->ReadFromFile(argv[1]);
  Game::Initialize(*game_params);
  unique_ptr<Params> base_card_params = CreateCardAbstractionParams();
  base_card_params->ReadFromFile(argv[2]);
  unique_ptr<CardAbstraction>
    base_card_abstraction(new CardAbstraction(*base_card_params));
  unique_ptr<Params> endgame_card_params = CreateCardAbstractionParams();
  endgame_card_params->ReadFromFile(argv[3]);
  unique_ptr<CardAbstraction>
    endgame_card_abstraction(new CardAbstraction(*endgame_card_params));
  unique_ptr<Params> merged_card_params = CreateCardAbstractionParams();
  merged_card_params->ReadFromFile(argv[4]);
  unique_ptr<CardAbstraction>
    merged_card_abstraction(new CardAbstraction(*merged_card_params));
  unique_ptr<Params> base_betting_params = CreateBettingAbstractionParams();
  base_betting_params->ReadFromFile(argv[5]);
  unique_ptr<BettingAbstraction>
    base_betting_abstraction(new BettingAbstraction(*base_betting_params));
  unique_ptr<Params> endgame_betting_params = CreateBettingAbstractionParams();
  endgame_betting_params->ReadFromFile(argv[6]);
  unique_ptr<BettingAbstraction>
    endgame_betting_abstraction(
		    new BettingAbstraction(*endgame_betting_params));
  unique_ptr<Params> base_cfr_params = CreateCFRParams();
  base_cfr_params->ReadFromFile(argv[7]);
  unique_ptr<CFRConfig>
    base_cfr_config(new CFRConfig(*base_cfr_params));
  unique_ptr<Params> endgame_cfr_params = CreateCFRParams();
  endgame_cfr_params->ReadFromFile(argv[8]);
  unique_ptr<CFRConfig>
    endgame_cfr_config(new CFRConfig(*endgame_cfr_params));
  unique_ptr<Params> merged_cfr_params = CreateCFRParams();
  merged_cfr_params->ReadFromFile(argv[9]);
  unique_ptr<CFRConfig>
    merged_cfr_config(new CFRConfig(*merged_cfr_params));

  unsigned int solve_street, base_it, endgame_it;
  if (sscanf(argv[10], "%u", &solve_street) != 1)  Usage(argv[0]);
  if (sscanf(argv[11], "%u", &base_it) != 1)       Usage(argv[0]);
  if (sscanf(argv[12], "%u", &endgame_it) != 1)    Usage(argv[0]);
  string m = argv[13];
  ResolvingMethod method;
  if (m == "unsafe")         method = ResolvingMethod::UNSAFE;
  else if (m == "cfrd")      method = ResolvingMethod::CFRD;
  else if (m == "maxmargin") method = ResolvingMethod::MAXMARGIN;
  else if (m == "combined")  method = ResolvingMethod::COMBINED;
  else                       Usage(argv[0]);

  BettingTree *base_betting_tree =
    BettingTree::BuildTree(*base_betting_abstraction);
  BettingTree *endgame_betting_tree =
    BettingTree::BuildTree(*endgame_betting_abstraction);
  BoardTree::Create();

  Assembler assembler(base_betting_tree, endgame_betting_tree, solve_street,
		      base_it, endgame_it, *base_card_abstraction,
		      *endgame_card_abstraction, *merged_card_abstraction,
		      *base_betting_abstraction, *endgame_betting_abstraction,
		      *base_cfr_config, *endgame_cfr_config,
		      *merged_cfr_config, method);
  assembler.Go();
}
