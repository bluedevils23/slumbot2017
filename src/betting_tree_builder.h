#ifndef _BETTING_TREE_BUILDER_H_
#define _BETTING_TREE_BUILDER_H_

#include <vector>

using namespace std;

class BettingAbstraction;
class Node;
// class Pool;
class Writer;

class BettingTreeBuilder {
public:
  BettingTreeBuilder(const BettingAbstraction &ba);
  BettingTreeBuilder(const BettingAbstraction &ba, unsigned int target_player);
  void Build(void);
  void Write(void);
  Node *CreateNoLimitTree1(unsigned int street, unsigned int pot_size,
			   unsigned int last_bet_size,
			   unsigned int num_street_bets, 
			   unsigned int player_acting,
			   unsigned int target_player,
			   unsigned int *terminal_id);
  Node *CreateNoLimitTree4(unsigned int street, unsigned int pot_size,
			   unsigned int last_bet_size,
			   unsigned int num_street_bets,
			   unsigned int player_acting,
			   unsigned int target_player,
			   const BettingAbstraction &
			   immediate_betting_abstraction,
			   const BettingAbstraction &
			   future_betting_abstraction,
			   unsigned int *terminal_id);
  Node *BuildAugmented(Node *base_node, Node *branch_point,
		       unsigned int new_bet_size, unsigned int num_street_bets,
		       unsigned int target_player,
		       unsigned int *num_terminals);
  
private:
  Node *CreateCallSucc(unsigned int street, unsigned int last_pot_size,
		       unsigned int last_bet_size,
		       unsigned int num_street_bets,
		       unsigned int player_acting, unsigned int target_player,
		       unsigned int *terminal_id);
  Node *CreateFoldSucc(unsigned int street, unsigned int last_pot_size,
		       unsigned int player_acting, unsigned int *terminal_id);
  void GetAllNewPotSizes(int old_pot_size, vector<int> *new_pot_sizes);
  bool AddGeometric1Bet(int old_pot_size, bool *pot_size_seen);
  bool AddGeometric2Bet(int old_pot_size, bool *pot_size_seen);
  void GetNewPotSizes(int old_pot_size, const vector<double> &pot_fracs,
		      unsigned int player_acting, unsigned int target_player,
		      bool *pot_size_seen);
  void HandleBet(unsigned int street, unsigned int last_bet_size,
		 unsigned int old_after_call_pot_size,
		 unsigned int new_after_call_pot_size,
		 unsigned int num_street_bets, unsigned int player_acting,
		 unsigned int target_player, unsigned int *terminal_id,
		 vector<Node *> *bet_succs);
  void CreateNoLimitSuccs(unsigned int street, unsigned int last_pot_size,
			  unsigned int last_bet_size,
			  unsigned int num_street_bets,
			  unsigned int player_acting,
			  unsigned int target_player,
			  unsigned int *terminal_id, Node **call_succ,
			  Node **fold_succ, vector<Node *> *bet_succs);
  Node *CreateNoLimitSubtree(unsigned int street, unsigned int pot_size,
			     unsigned int last_bet_size,
			     unsigned int num_street_bets, 
			     unsigned int player_acting,
			     unsigned int target_player,
			     unsigned int *terminal_id);
  void CreateLimitSuccs(unsigned int street, unsigned int pot_size,
			unsigned int last_bet_size, unsigned int num_bets,
			unsigned int last_bettor, unsigned int player_acting,
			unsigned int *terminal_id, Node **call_succ,
			Node **fold_succ, vector<Node *> *bet_succs);
  Node *CreateLimitSubtree(unsigned int street, unsigned int pot_size,
			   unsigned int last_bet_size, unsigned int num_bets,
			   unsigned int last_bettor, unsigned int player_acting,
			   unsigned int *terminal_id);

  void GetNewPotSizes(int old_pot_size, const vector<int> &bet_amounts,
		      unsigned int player_acting, unsigned int target_player,
		      vector<int> *new_pot_sizes);
  Node *CreateCallSucc2(unsigned int street, unsigned int last_pot_size,
			unsigned int last_bet_size,
			unsigned int num_street_bets,
			unsigned int num_bets, bool p1_last_bettor,
			unsigned int player_acting, unsigned int target_player,
			unsigned int *terminal_id);
  void HandleBet2(unsigned int street, unsigned int last_bet_size,
		  unsigned int old_after_call_pot_size,
		  unsigned int new_after_call_pot_size,
		  unsigned int num_street_bets, unsigned int num_bets,
		  unsigned int player_acting, unsigned int target_player,
		  unsigned int *terminal_id, vector<Node *> *bet_succs);
  void CreateNoLimitSuccs2(unsigned int street, unsigned int last_pot_size,
			   unsigned int last_bet_size,
			   unsigned int num_street_bets, unsigned int num_bets,
			   bool p1_last_bettor, unsigned int player_acting,
			   unsigned int target_player,
			   unsigned int *terminal_id, Node **call_succ,
			   Node **fold_succ, vector<Node *> *bet_succs);
  Node *CreateNoLimitSubtree2(unsigned int street, unsigned int pot_size,
			      unsigned int last_bet_size,
			      unsigned int num_street_bets,
			      unsigned int num_bets, bool p1_last_bettor,
			      unsigned int player_acting,
			      unsigned int target_player,
			      unsigned int *terminal_id);
  Node *CreateNoLimitTree2(unsigned int street, unsigned int pot_size,
			   unsigned int last_bet_size,
			   unsigned int num_street_bets, unsigned int num_bets,
			   bool p1_last_bettor, unsigned int player_acting,
			   unsigned int target_player,
			   unsigned int *terminal_id);

  // Methods from no_limit_tree3.cpp
  bool CloseToAllIn(int old_pot_size);
  bool GoGeometric(int old_pot_size);
  void AddFracBet(int old_pot_size, double frac, bool *pot_size_seen);
  void AddOurPreflopBet(int old_pot_size, unsigned int player_acting,
			bool *pot_size_seen, unsigned int num_street_bets);
  void AddOurFlopBet(int old_pot_size, bool *pot_size_seen,
		     unsigned int num_street_bets);
  void AddOurTurnBet(int old_pot_size, bool *pot_size_seen,
		     unsigned int num_street_bets);
  void AddOurRiverBet(int old_pot_size, bool *pot_size_seen);
  void AddOppPreflopBet(int old_pot_size, bool *pot_size_seen);
  void AddOppFlopBet(int old_pot_size, bool *pot_size_seen,
		     unsigned int num_street_bets);
  void AddOppTurnBet(int old_pot_size, bool *pot_size_seen,
		     unsigned int num_street_bets);
  void AddOppRiverBet(int old_pot_size, bool *pot_size_seen,
		      unsigned int num_street_bets);
  void AddBets(unsigned int st, unsigned int player_acting,
	       unsigned int target_player, int old_pot_size,
	       unsigned int num_street_bets, unsigned int last_bet_size,
	       vector<Node *> *bet_succs, unsigned int *terminal_id);
  Node *CreateCallSucc3(unsigned int street, unsigned int last_pot_size,
			unsigned int last_bet_size,
			unsigned int num_street_bets, bool p1_last_bettor,
			unsigned int player_acting, unsigned int target_player,
			unsigned int *terminal_id);
  void CreateNoLimitSuccs3(unsigned int street, unsigned int last_pot_size,
			   unsigned int last_bet_size,
			   unsigned int num_street_bets, bool p1_last_bettor,
			   unsigned int player_acting,
			   unsigned int target_player,
			   unsigned int *terminal_id, Node **call_succ,
			   Node **fold_succ, vector<Node *> *bet_succs);
  Node *CreateNoLimitSubtree3(unsigned int street, unsigned int pot_size,
			      unsigned int last_bet_size,
			      unsigned int num_street_bets,
			      bool p1_last_bettor, unsigned int player_acting,
			      unsigned int target_player,
			      unsigned int *terminal_id);
  Node *CreateNoLimitTree3(unsigned int street, unsigned int pot_size,
			   unsigned int last_bet_size,
			   unsigned int num_street_bets, bool p1_last_bettor,
			   unsigned int player_acting,
			   unsigned int target_player,
			   unsigned int *terminal_id);
  
  void GetNewPotSizes4(unsigned int old_pot_size,
		       const vector<double> &pot_fracs,
		       unsigned int player_acting, unsigned int target_player,
		       const BettingAbstraction &betting_abstraction,
		       vector<int> *new_pot_sizes);
  void HandleBet4(unsigned int street, unsigned int last_bet_size,
		  unsigned int old_after_call_pot_size,
		  unsigned int new_after_call_pot_size,
		  unsigned int num_street_bets, unsigned int player_acting,
		  unsigned int target_player,
		  const BettingAbstraction &immediate_betting_abstraction,
		  const BettingAbstraction &future_betting_abstraction,
		  unsigned int *terminal_id, vector<Node *> *bet_succs);
  Node *CreateCallSucc4(unsigned int street, unsigned int last_pot_size,
			unsigned int last_bet_size,
			unsigned int num_street_bets,
			unsigned int player_acting, unsigned int target_player,
			const BettingAbstraction &immediate_betting_abstraction,
			const BettingAbstraction &future_betting_abstraction,
			unsigned int *terminal_id);
  void CreateNoLimitSuccs4(unsigned int street, unsigned int last_pot_size,
			   unsigned int last_bet_size,
			   unsigned int num_street_bets,
			   unsigned int player_acting,
			   unsigned int target_player,
			   const BettingAbstraction &
			   immediate_betting_abstraction,
			   const BettingAbstraction &
			   future_betting_abstraction,
			   unsigned int *terminal_id, Node **call_succ,
			   Node **fold_succ, vector<Node *> *bet_succs);
  Node *CreateNoLimitSubtree4(unsigned int street, unsigned int pot_size,
			      unsigned int last_bet_size,
			      unsigned int num_street_bets,
			      unsigned int player_acting,
			      unsigned int target_player,
			      const BettingAbstraction &
			      immediate_betting_abstraction,
			      const BettingAbstraction &
			      future_betting_abstraction,
			      unsigned int *terminal_id);
  
  void Initialize(void);
  void Write(Node *node, Writer *writer);

  const BettingAbstraction &betting_abstraction_;
  bool asymmetric_;
  unsigned int target_player_;
  unsigned int initial_street_;
  unsigned int stack_size_;
  unsigned int all_in_pot_size_;
  unsigned int min_bet_;
  // Pool *pool_;
  Node *root_;
  unsigned int num_terminals_;
};

#endif