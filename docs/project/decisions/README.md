# Decisions (ADRs)

| ADR | Decision | Status |
| --- | --- | --- |
| [0001](ADR-0001-fork-felucca-and-keep-t1.md) | Build on Felucca 1.6 + the T-1 fork, then reduce | accepted |
| [0002](ADR-0002-backlog-in-repo.md) | Backlog as markdown in the repo, run by subagents | accepted |
| [0003](ADR-0003-sequencer-core.md) | Sequencer core architecture and algorithm interface (pure `build`/`step`, table-driven layers/morphs, 192 PPQN scheduler, RAM budget) | proposed (P2-004, awaiting owner) |
| [0004](ADR-0004-design-lock.md) | Design lock before development, plus the flexible-by-design addendum | accepted |
| [0005](ADR-0005-preset-flash-map.md) | Preset and settings flash map: 16 A/B slots at 0xA0000..0xBFFFF, settings stay at 0xFC000 | proposed (P7-001, awaiting owner) |
| [0006](ADR-0006-screen-architecture-v2.md) | Screen architecture v2: scaling glance screen, message slot, keybed footer | accepted |
| [0007](ADR-0007-algorithm-ui-descriptor.md) | Algorithm UI descriptor (amends ADR-0003 AlgoDesc/MorphDesc) | accepted |

Template: context, decision, consequences. Number sequentially; never renumber. A superseded ADR stays and links to
its replacement.
