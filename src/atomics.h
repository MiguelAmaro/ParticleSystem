#ifndef ATOMICS_H
#define ATOMICS_H

u64 AtomicExchange(volatile u64 *Var, u64 SetTo)
{
  return InterlockedExchange64((volatile LONG64 *)Var, SetTo);
}

#endif //ATOMICS_H
