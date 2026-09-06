# ShadowNet

ShadowNet is a single-player idle / tycoon game for Windows, written in C++ against the raw Win32 API. You play an up-and-coming operator in a fictional hacker underground, running jobs, training skills, hiring a crew, managing "heat" from law enforcement, and climbing from Script Kiddie to Shadow Broker. All of the "hacking" is narrative flavor and simulated numbers. There is no real network or system activity of any kind.

The tone is a love letter to 90s BBS / IRC hacker culture: neon-on-black terminal styling, ANSI-art references, pop-up IRC chatter, and a dramatic full-screen "screen melt" when someone turns the tables on you.

## Table of contents

- [Building and running](#building-and-running)
- [Project layout](#project-layout)
- [Core loop at a glance](#core-loop-at-a-glance)
- [Gameplay systems](#gameplay-systems)
- [Progression and decision paths](#progression-and-decision-paths)
- [Save system](#save-system)
- [Credits and disclaimer](#credits-and-disclaimer)

## Building and running

This is a Visual Studio C++ desktop project.

Requirements:

- Visual Studio 2022 (v143 toolset) with the "Desktop development with C++" workload
- Windows 10 or 11

To build:

1. Open `ShadowNet.sln` in Visual Studio.
2. Pick a configuration (`Debug` or `Release`) and the `x64` platform.
3. Build the solution (Ctrl+Shift+B).

Or from a Developer Command Prompt:

```
msbuild ShadowNet.sln /p:Configuration=Release /p:Platform=x64
```

The output binary lands in `bin\Debug\ShadowNet.exe` (or the matching Release folder). The app is a standard windowed Win32 program with an icon compiled in from `ShadowNet.rc` / `media\ghost.ico`. It links only against system libraries (`user32`, `gdi32`, `shell32`), so there is nothing to install.

The game saves to `%APPDATA%\ShadowNet\save.dat` and autosaves roughly once a minute, so your progress carries across runs automatically.

## Project layout

```
src\
  core\      main.cpp        Win32 entry point, message loop, window procs, input routing
             game.cpp/.h     GameState struct, the per-second GameTick, job start/complete,
                             military route, init, all the core constants and enums
  systems\
             jobs.cpp        Static catalog: 103 jobs, 9 documentation entries, 4 boss ops
             skills.cpp      Speed / payout bonus math, multitasking penalties
             intel.cpp       Underground intel market: time-reduction perks, stock refresh
             events.cpp      Random world events (raids, crashes, tips, betrayals)
             actions.cpp     Hiring, market buying, prestige activation
             offline.cpp     Offline-progress catch-up applied on load
             save.cpp        Binary save / load to %APPDATA%
  windows\   irc_window      Pop-up IRC chatter for hires and job unlocks
             boss_window      Animated multi-stage "boss hack" terminal
             jail_window      Full-screen lockout while seized by the feds
             melt_window      Full-screen glitch / matrix "you got hacked" effect
  ui\        ui.cpp/.h       All tab rendering, layout, click handling, colors.h palette
             hacker_stage    Animated pixel-art hacker strip above the panel and in the
                             minimized overlay; reacts to jobs, paydays, heat, raids, jail
  data\      strings.cpp/.h  Hacker handles, IRC line pools, intel item definitions
```

The game holds everything in a single flat `GameState` struct (see `src/core/game.h`). There is no dynamic game-object graph: jobs, skills, personnel, and inventory are all fixed-size arrays inside that struct, which is what makes the binary save format a straight memory dump.

## Core loop at a glance

`GameTick` in `src/core/game.cpp` runs once per second and drives everything:

1. If jailed, skip all gameplay except autosave until the sentence expires.
2. If enlisted in the military route, run only the service processing and return.
3. Tick down every active job; complete any that hit zero.
4. Age personnel: run down arrest timers, decay loyalty over time.
5. Decay heat on a timer (faster with Forensic Evasion skill).
6. Roll random events.
7. Fluctuate market prices.
8. Recompute unlocks (job slots, personnel slots, newly available jobs).
9. Expire temporary event buffs, doc study, mentorship, boss cooldowns, ban timers.
10. Blink the cursor, autosave every 60 ticks.
11. If heat hit 100, trigger a federal seizure (jail).

## Gameplay systems

**Jobs.** The heart of the game. 103 jobs across four tiers (Tier 4 Script Kiddie up to Tier 1 APT / Shadow Broker), plus three parallel job families: legitimate prestige-mode jobs, game-hacking jobs, and military-contractor jobs. Each job has a base time, payout, reputation reward, heat generated, skills it trains, skill requirements, an optional required documentation, and an optional prerequisite job with a completion count. Jobs unlock automatically once every condition is satisfied.

**Skills.** 17 skills (15 core plus Art and AI/ML). Running jobs grants XP; leveling follows a quadratic curve that turns exponential past level 50, capping at 100. Skill levels feed job speed and payout bonuses, with "scarcity" step bonuses at levels 25 / 50 / 75 / 100. AI/ML is a deliberate double-edged skill: it speeds jobs but taxes XP gain. Specializing deep in one skill is rewarded over spreading thin (a specialization-depth penalty).

**Documentation study.** Nine reference works you study to unlock job categories and earn passive skill XP. Some are free, some must be purchased. Each doc can be studied up to 100 passes for escalating mastery tiers, with diminishing XP per pass.

**Personnel.** Hire up to 15 crew members (13 types, from Script Kiddie to Infrastructure Architect), gated by cost and reputation. When you start a job, an idle crew member is auto-assigned to help, preferring one whose specialty matches the job. Assigned crew run jobs faster; a non-specialist takes a payout cut, but a specialist working a job in their field keeps full value. Crew level up, slowly lose loyalty, and can be arrested in raids or defect if loyalty drops too low. Trying to hire someone you cannot afford triggers a scam, and doing it too many times gets you hacked.

**Markets.** Two of them. The regular **Market** has a buy side and a sell side. You buy tools and infrastructure (VPN and proxy chain for heat reduction, burner phones, dedicated servers, bulletproof VPS, Tor hidden service, and the Satellite Uplink for passive offline income; hardware, zero-day, and C2 kits that speed up their matching job types). You sell stolen goods you pick up along the way, such as data dumps from high-tier data-theft jobs and spare exploits. The **Intel** underground market sells data dumps, access, and tooling that shave time off matching jobs.

**Heat and law enforcement.** Every dirty job raises heat (0 to 100). Operating without protective gear compounds heat faster each consecutive job. High heat unlocks escalating random events: investigations, rival attacks, FBI raids, and Interpol dragnets. Hitting 100 heat, or catching a federal indictment, seizes your operation and jails you.

**Random events.** Roughly every ten seconds the game rolls on a weighted table: forum tips, law-enforcement heat, rival hacker theft, FBI / Interpol actions, data-breach news (price spikes), Patch Tuesday, crypto crashes and booms, whistleblower betrayals, underground alliances, free zero-day drops, competitor arrests, journalist investigations, and federal indictments.

**Boss operations.** Four scripted, animated "boss hacks" gate the tier jumps (T4 to T3, T3 to T2, T2 to T1, and T1 to Shadow Broker). Each plays out as a multi-stage terminal sequence with per-stage skill checks; you can buy prep bonuses beforehand to improve your odds. Success unlocks the next tier; failure halves your reputation, seizes your cash, degrades your skills, and puts the op on cooldown.

**Military cyber route.** An optional four-day service path that opens after the first boss. It suspends civilian operations, force-trains all skills to 65 percent, lets you pick one specialty to push to 100, pays a steady salary, and on completion makes you a Cyber Veteran: all tiers unlocked and the contractor job family available.

**Jail.** Being seized locks you out for a real-world day (two days for a contractor burn). The full-screen jail window counts down; offline time counts against the sentence.

**Screen melt.** When a scorned scammer or a disloyal crew member turns on you, the whole screen glitches, melts, and rains matrix code with the attacker's handle. Pure spectacle, no lasting mechanical penalty beyond the heat and reputation hit that triggered it.

**Hacker stage.** A pixel-art strip above the gameplay panel shows your hacker at their desk: head bobbing, typing while jobs run, reading while studying docs, fist-pumping on a payday, recoiling from raids and investigations, and hands-up when seized. Heat adds sweat drops and, near the top, a siren pulse. While enlisted the scene swaps to a windowless military cubicle in uniform. The same scene appears at half size in the minimized overlay. It only observes game state; nothing about it is saved.

**Prestige.** Once you clear the endgame requirements (10,000 reputation, 5 million cash, and five Tier 1 jobs completed) you can prestige: you keep a permanent bonus, wipe your progress, and switch into legitimate-consultancy mode with a parallel catalog of white-hat jobs.

**Offline progress.** On load, the game catches up on elapsed real time (capped at 8 hours): finishing running jobs, decaying heat, releasing arrested crew, accruing passive infrastructure income, advancing doc study, and progressing military service.

## Progression and decision paths

A typical run branches like this:

1. **Script Kiddie (Tier 4).** Start with a few gateway jobs unlocked. Grind low jobs, study free docs, and inch reputation up while keeping heat manageable. Buy a VPN or burner early to stop heat compounding.
2. **Tier gate: Operation Marketplace.** Clear the first boss op to unlock Tier 3. This is also where the game offers a fork.

At the first tier gate the path splits into three long-term directions that are worth calling out, because they change how the rest of the run plays:

- **The criminal ladder (default).** Keep pushing dirty jobs up through Tiers 3, 2, and 1, clearing a boss op at each gate, hiring a bigger crew, and buying infrastructure. This is the highest-ceiling path and the one that leads to Shadow Broker and prestige.
- **The military route.** After the first boss you can enlist. You trade roughly four real days and all civilian activity for guaranteed skills (everything to 65, one specialty to 100), a fixed salary, all tiers unlocked, and access to the high-paying but high-risk contractor jobs. It is the safe, steady, skills-first detour.
- **Going legitimate (prestige).** The long game. Reach the endgame thresholds, then prestige into white-hat consulting. You reset but carry a permanent multiplier and a whole separate legitimate job catalog.

3. **Managing the squeeze.** Throughout, the central tension is payout versus heat. Higher tiers pay more but generate more heat, and the law-enforcement event table gets nastier as heat climbs. Protective tools, OPSEC skill, Forensic Evasion, and OPSEC-specialist crew all bleed heat back down. Push too hard and you get jailed, which costs you a real-world day.
4. **The endgame.** Clear the final boss (Operation Ghost Protocol) to become the Shadow Broker, then decide whether to keep accumulating or prestige and start the legitimate ladder.

Two smaller decision loops sit inside all of this: **hiring** (each hire pops an IRC window that is either a genuine recruit or a scam, and repeated scams escalate to a hack), and **boss prep** (spend cash and time on prep bonuses before an op, or gamble on your raw skill checks).

## Save system

Saves are a versioned binary dump of the `GameState` fields to `%APPDATA%\ShadowNet\save.dat`. The format is currently version 6; the loader understands versions 1 through 6 and fills in newer fields with sensible defaults when loading an older save. A magic number (`SNET`) guards the header. Autosave fires every 60 ticks, on jail, and on exit. Deleting the file (or using the in-game reset) starts a fresh game.

## Credits and disclaimer

ShadowNet is a work of fiction and a nostalgia piece. The job names, "operations," breach references, and tooling names are set dressing for a tycoon game. Nothing in this project performs, teaches, or enables any real intrusion, and the code touches nothing outside its own window and its own save file.
