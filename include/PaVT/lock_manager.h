#ifndef PAVT_LOCK_MANAGER_H
#define PAVT_LOCK_MANAGER_H

#include <stack>

#include "PaVT/Base/locknode.h"

namespace pavt {

class LockManager {
 public:
  LockManager();
  ~LockManager();
  void Lock(base::LockNode* node);
  bool TryLock(base::LockNode* node);
  base::LockNode* Unlock();
  void UnlockAll();
 protected:
  std::stack<base::LockNode*> _lock_stack;
};
} // pavt
#endif // PAVT_LOCK_MANGER_H_