#include <stdio.h>
#include <stdlib.h>

#include "betting_abstraction.h"
#include "betting_tree.h"
#include "betting_tree_builder.h"
#include "files.h"
#include "game.h"
#include "io.h"

void BettingTreeBuilder::Build(void) {
  unsigned int terminal_id = 0;

  if (Game::NumPlayers() > 2) {
    root_ = CreateMPTree(target_player_, &terminal_id);
  } else if (betting_abstraction_.Limit()) {
    root_ = CreateLimitTree(&terminal_id);
  } else {
    if (betting_abstraction_.NoLimitTreeType() == 0) {
    } else if (betting_abstraction_.NoLimitTreeType() == 1) {
      root_ = CreateNoLimitTree1(target_player_, &terminal_id);
    } else if (betting_abstraction_.NoLimitTreeType() == 2) {
      root_ = CreateNoLimitTree2(target_player_, &terminal_id);
    }
  }
  num_terminals_ = terminal_id;
}

// To handle reentrant trees we keep a boolean array of nodes that have
// already been visited.  Note that even if a node has already been visited,
// we still write out the properties of the node (ID, flags, pot size, etc.).
// But we prevent ourselves from redundantly writing out the subtree below
// the node more than once.
void BettingTreeBuilder::Write(Node *node, unsigned int **num_nonterminals,
			       Writer *writer) {
  unsigned int st = node->Street();
  unsigned int id = node->ID();
  unsigned int pa = node->PlayerActing();
  bool nt_first_seen = (id == kMaxUInt);
  // Assign IDs during writing
  if (nt_first_seen) {
    id = num_nonterminals[pa][st]++;
    node->SetNonterminalID(id);
  }
  writer->WriteUnsignedInt(id);
  writer->WriteUnsignedShort(node->LastBetTo());
  writer->WriteUnsignedShort(node->NumSuccs());
  writer->WriteUnsignedShort(node->Flags());
  writer->WriteUnsignedChar(pa);
  writer->WriteUnsignedChar(node->NumRemaining());
  if (node->Terminal()) {
    return;
  }
  if (! nt_first_seen) return;
  unsigned int num_succs = node->NumSuccs();
  for (unsigned int s = 0; s < num_succs; ++s) {
    Write(node->IthSucc(s), num_nonterminals, writer);
  }
}

void BettingTreeBuilder::Write(void) {
  char buf[500];
  if (asymmetric_) {
    sprintf(buf, "%s/betting_tree.%s.%u.%s.%u", Files::StaticBase(),
	    Game::GameName().c_str(), Game::NumPlayers(),
	    betting_abstraction_.BettingAbstractionName().c_str(),
	    target_player_);
  } else {
    sprintf(buf, "%s/betting_tree.%s.%u.%s", Files::StaticBase(),
	    Game::GameName().c_str(), Game::NumPlayers(),
	    betting_abstraction_.BettingAbstractionName().c_str());
  }
  
  unsigned int max_street = Game::MaxStreet();
  unsigned int num_players = Game::NumPlayers();
  unsigned int **num_nonterminals = new unsigned int *[num_players];
  for (unsigned int pa = 0; pa < num_players; ++pa) {
    num_nonterminals[pa] = new unsigned int[max_street + 1];
    for (unsigned int st = 0; st <= max_street; ++st) {
      num_nonterminals[pa][st] = 0;
    }
  }
  
  Writer writer(buf);
  Write(root_.get(), num_nonterminals, &writer);
  for (unsigned int st = 0; st <= max_street; ++st) {
    unsigned int sum = 0;
    for (unsigned int pa = 0; pa < num_players; ++pa) {
      sum += num_nonterminals[pa][st];
    }
    fprintf(stderr, "St %u num nonterminals %u\n", st, sum);
  }
  for (unsigned int pa = 0; pa < num_players; ++pa) {
    delete [] num_nonterminals[pa];
  }
  delete [] num_nonterminals;
}


void BettingTreeBuilder::Initialize(void) {
  initial_street_ = betting_abstraction_.InitialStreet();
  stack_size_ = betting_abstraction_.StackSize();
  all_in_pot_size_ = 2 * stack_size_;
  min_bet_ = betting_abstraction_.MinBet();

  // pool_ = new Pool();
  root_ = NULL;
  num_terminals_ = 0;
}

BettingTreeBuilder::BettingTreeBuilder(const BettingAbstraction &ba) :
  betting_abstraction_(ba) {
  asymmetric_ = false;
  // Parameter should be ignored for symmetric trees.
  target_player_ = kMaxUInt;
  node_map_.reset(new
		  unordered_map< unsigned long long int, shared_ptr<Node> >);
    
  Initialize();
}

BettingTreeBuilder::BettingTreeBuilder(const BettingAbstraction &ba,
				       unsigned int target_player) :
  betting_abstraction_(ba) {
  asymmetric_ = true;
  target_player_ = target_player;
  Initialize();
}

