<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function f()[local]: int {
  if (Rx\IS_ENABLED) {
    return rx();
  } else {
    return nonrx();
  }
}

<<__Rx>>
function rx()[rx]: int {
  return 1;
}

function nonrx(): int {
  return 1;
}
