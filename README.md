# wave spam helper v0.1.0 source alpha

direction-aware spam smoothing, not a straight-line lock

## normal mode support

works in normal and practice modes, there is no practice-mode requirement and enable never switches modes

this is assisted gameplay, normal progress saving and leaderboard submission are not blocked by this alpha, do not present assisted runs as unassisted completions or records

## how it works

recent hold and release cycles estimate your spam speed and hold balance

roughly equal holds and releases produce roughly level wave travel under ordinary conditions

consistently longer holds bias travel toward the held wave direction, longer releases bias it the other way, gravity inversion reverses the world direction

the predictor smooths alternating timing jitter while following sustained changes in hold balance

it cannot read your intentions, route through hazards or know whether drifting was deliberate

only received input edges are scheduled, it does not generate an ongoing stream of clicks when you stop

## latency tradeoff

correcting an input to an earlier point in the smoothed output stream requires buffering the stream first

by default detected spam receives a 25 ms base buffer, prediction can adjust that delay within a bounded range

this is not zero-latency assistance, actual per-edge latency varies, dense inputs and physics tick rounding can add delay

## use

install a successful windows build, enter a level and pause

click spam to open the panel, then enable

start with 85 percent strength and a 25 ms buffer

it learns from at least two complete cycles in the 6 to 40 cps range before smoothing

outside wave mode, while disabled or at zero strength, inputs pass through

long holds or stopped spam leave the learned rhythm after an adaptive timeout, queued assistance is cancelled and physical button state is restored

use off if it fights your steering, changing strength or buffer also cancels queued assistance and relearns the rhythm

player 1 and player 2 have independent engines, the hud currently displays player 1 details

a small assisted indicator remains visible when enabled in wave mode even if detailed diagnostics are hidden

no macros or external services are needed

## testing status

standalone engine compiled locally with c++23, address and undefined behaviour sanitizers, warnings as errors

100000 bypass events preserved, 1000 randomized alternating bursts preserved edge order and counts within the tested latency bound

a synthetic alternating duty-jitter test reduced rms duty error from 0.13 to about 0.0124, this is a controlled engine test, not a claim about gd success rates

sustained steering tests converged toward 25 percent and 75 percent hold ratios

reset, inactive mode, zero strength, no-input and long-hold timeout checks passed

## not yet verified

the gd adapter and popup have not been compiled locally or tested in gd, github actions is the first sdk build gate

real wave physics, mini wave, inversion, portals, checkpoints, input hook timing and conflicts with cbf or other mods need in-game testing

this first alpha may still feel delayed or misclassify intentional changes as jitter, keep the buffer small and stop using assistance if it makes control worse

## build and download

push these files to the root of the alt-spam-helper repo

the windows alpha workflow builds for gd 2.2081 and geode 5.8.2 and uploads alt-spam-helper-windows

extract its artifact zip to obtain geekedgdplayer.alt-spam-helper.geode

this source zip itself is not an installable mod

## native tests

```sh
g++ -std=c++23 -Wall -Wextra -Werror -fsanitize=address,undefined -Isrc tests/SpamSmootherTest.cpp src/engine/SpamSmoother.cpp -o spam-tests
./spam-tests
python3 tools/check-comments.py
```
