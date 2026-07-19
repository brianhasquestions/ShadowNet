#include "strings.h"

// ============================================================================
// Hacker Usernames (500+ for IRC chat dialogues)
// Includes famous hackers, scene handles, and generated names
// ============================================================================

extern const wchar_t* g_HackerUsernames[] = {
    // Famous/Notorious hackers & aliases
    L"DreadPirateRoberts", L"Z3r0_C00L", L"Phiber_Optik", L"Pryx",
    L"Acid_Burn", L"CrashOverride", L"The_Mentor", L"Captain_Crunch",
    L"Dark_Dante", L"Mafiaboy", L"c0mrade", L"Mitnick",
    L"Stoll_Hunter", L"Solar_Sunrise", L"MafiaBoy2k", L"GucciferRO",
    L"Guccifer_2_0", L"Sabu_LulzSec", L"Topiary_", L"tflow_",
    L"Kayla_IRC", L"Avunit_", L"Anarchaos_", L"Weev_",
    L"HectorX", L"AnonOps_", L"JesterActual", L"th3j35t3r",
    L"LulzSec_", L"AntiSec_", L"LizardSquad_", L"PhantomSquad_",
    L"TeaMp0isoN_", L"CabinCr3w", L"UGNazi_", L"NullCrew_",
    L"GhostShell_", L"Rex_Mundi", L"Darkode_Admin", L"Peace_of_Mind",
    L"1x0123", L"IntelBroker", L"ShinyHunters", L"USDoD_",
    L"SubVirt", L"LockBit_Rep", L"Bl4ck_B4st4", L"ALPHV_Admin",
    // L33t speak / classic scene handles
    L"zer0_c00l", L"n3on_gh0st", L"d4rk_fl0w", L"r00t_sh3ll",
    L"crypt0_k1d", L"h4x_m4ster", L"byt3_w1z", L"sk3l3t0n_k3y",
    L"ph4nt0m_r1d3r", L"c0d3_m0nk3y", L"d3ad_p1x3l", L"sh4d0w_0p",
    L"gl1tch_k1ng", L"v0id_w4lk3r", L"bl4ck_h4t", L"null_p0int3r",
    L"st4ck_0v3rfl0w", L"buff3r_0v3rrun", L"h34p_spr4y", L"r0p_ch41n",
    L"k3rn3l_p4n1c", L"s3gf4ult", L"r4c3_c0nd1t10n", L"d3ref_null",
    L"xss_k1ng", L"sqli_m4ster", L"rce_hunt3r", L"csrf_d3m0n",
    L"ssrf_sl4y3r", L"idor_f1nd3r", L"lfi_r34d3r", L"rfi_l04d3r",
    // Network / infra themed
    L"bgp_h1j4ck", L"dns_p01s0n", L"arp_sp00f", L"syn_fl00d",
    L"tcp_r3s3t", L"udp_4mplify", L"icmp_tun3l", L"vlan_h0pp3r",
    L"stp_4tt4ck", L"ospf_p01s0n", L"eigrp_sp00f", L"mpls_p1v0t",
    L"wifi_pwn3r", L"wpa2_cr4ck", L"evil_tw1n", L"karma_att4ck",
    L"rogue_4p", L"deauth_k1ng", L"pmkid_hunt", L"handshake_4x",
    L"nmap_sc4n", L"masscan_", L"zmap_sw33p", L"shodan_b0t",
    L"censys_crawl", L"nuclei_sc4n", L"subfind3r", L"amass_enum",
    // Windows / malware themed
    L"ntlm_r3l4y", L"kerber0ast", L"golden_t1ck3t", L"silver_t1ck3t",
    L"mimik4tz", L"rubbe_hose", L"c0b4lt_str1ke", L"m3tasploit",
    L"psex3c_", L"wmi_l4t3r4l", L"dcom_exec", L"smbgh0st",
    L"eternal_blu3", L"doublepuls4r", L"wannacry_", L"notpety4",
    L"conti_l34k", L"ryuk_d3pl0y", L"lockbit_", L"blackc4t",
    L"revil_0p", L"darkside_", L"cl0p_gang", L"akira_r4ns0m",
    L"emotet_l04d", L"qakb0t_", L"trickb0t_", L"bazarl04d3r",
    L"cobalt_b34c0n", L"sliv3r_c2", L"mythic_c2", L"hav0c_c2",
    L"brut3r4t3l_", L"nim_pl4nt", L"rust_impl4nt", L"go_b34c0n",
    // UNIX / Linux themed
    L"sudo_rm_rf", L"chmod_777", L"chroot_j41l", L"iptabl3s",
    L"cron_j0b", L"ssh_tun3l", L"scp_exfil", L"tar_czf",
    L"grep_r3curs3", L"sed_4wk", L"bash_0n3lin3r", L"perl_r3v",
    L"python_p0c", L"ruby_spl01t", L"node_rce", L"php_sh3ll",
    L"websh3ll_", L"c99_sh3ll", L"r57_sh3ll", L"b374k_sh3ll",
    L"linp34s_", L"priv3sc_", L"dirty_p1p3", L"dirty_c0w",
    L"ptrace_0", L"cap_s3tuid", L"suid_hunt", L"cgroup_esc4p3",
    // Social engineering / OSINT themed
    L"pretxt3r_", L"vish3r_", L"smish3r_", L"baiter_",
    L"dumpst3r_div3", L"t41lg4t3r", L"shoulder_surf", L"quid_pr0_qu0",
    L"maltego_pr0", L"recon_ng_", L"theharv3st3r", L"osint_fr4mework",
    L"sherl0ck_", L"holehe_ch3ck", L"social_catf1sh", L"deep_f4k3r",
    L"voice_cl0n3", L"ai_phish_", L"gpt_lur3", L"llm_p01s0n",
    // Crypto / financial themed
    L"mixe_tumbl3r", L"ch41n_h0pp3r", L"flash_l04n", L"rug_pull3r",
    L"sandwich_b0t", L"mev_hunt3r", L"front_runn3r", L"defi_drain3r",
    L"bridge_h4ck3r", L"oracle_m4n1p", L"reentranc3_", L"selfdestruct_",
    L"private_k3y_", L"seed_phras3", L"hot_wall3t", L"cold_st0r4g3",
    L"monero_m1n3r", L"coinjoin_", L"wasabi_mix", L"tornado_c4sh",
    // Hardware / embedded themed
    L"jtag_d3bug", L"swd_pr0b3", L"uart_sh3ll", L"spi_fl4sh",
    L"i2c_sniff", L"glitch3r_", L"fault_inj3ct", L"side_ch4nn3l",
    L"emfi_puls3", L"laser_f1", L"chip_wh1sp3r3r", L"nand_dump",
    L"firmware_r3v", L"bootl04d3r_", L"secure_b00t", L"tpm_att4ck",
    L"rfid_cl0n3", L"nfc_r3l4y", L"proxm4rk3", L"flipper_z3r0",
    L"sdr_h4ck", L"gnur4dio", L"hackrf_0n3", L"rtl_sdr_",
    // Defensive / blue team (turned dark)
    L"soc_4n4lyst", L"ir_r3sp0nd3r", L"threat_hunt3r", L"mal_4n4lyst",
    L"yara_rul3", L"sigma_rul3", L"splunk_spe", L"elastic_qu3ry",
    L"wireshark_pr0", L"volatil1ty_", L"autopsy_f0r3nsic", L"sleuthk1t",
    L"ghidra_r3v", L"ida_pr0", L"x64dbg_", L"ollydbg_",
    L"binary_n1nj4", L"hopp3r_r3v", L"r2_pip3", L"cutter_r3v",
    // Misc cool handles
    L"n1ghtcr4wl3r", L"d4rkm4tt3r", L"qu4ntum_3rr0r", L"s1ngul4r1ty",
    L"bl4ck_h0l3", L"event_h0r1z0n", L"sch0rd1ng3r", L"3ntr0py_m4x",
    L"ch40s_th30ry", L"fr4ct4l_n01s3", L"n3ur4l_n3t", L"b4ckd00r_m4n",
    L"z3r0d4y_", L"n_d4y_", L"1_d4y_", L"p4tch_g4p",
    L"expl01t_db", L"cve_hunt3r", L"bug_b0unty", L"p0c_0r_gtf0",
    L"full_d1scl0s3", L"r3sp0ns1bl3_", L"c00rd1n4t3d_", L"vu1n_r3p0rt",
    L"cyber_r4ng3r", L"d1g1t4l_n0m4d", L"ghost_1n_w1r3s", L"matrix_r3al",
    L"n30_", L"morph3us_", L"trin1ty_", L"cyph3r_",
    L"sw1tch_", L"m0us3_", L"d0z3r_", L"ap0c_",
    L"null_byt3", L"d34d_dr0p", L"c0v3rt_ch4n", L"s1d3_ch4nn3l",
    L"timing_4tt4ck", L"p4dding_0r4cl3", L"bl33d1ng_b1t", L"h34rtbl33d",
    L"sh3llsh0ck_", L"l0g4sh3ll", L"spr1ng4sh3ll", L"str0ng_pwn",
    L"p4ssw0rd_spr4y", L"cred_stuff_", L"br00t3_f0rc3", L"r41nb0w_t4bl3",
    L"h4sh_c4t_", L"j0hn_r1pp3r", L"0phcr4ck_", L"hydra_att4ck",
    L"burp_su1t3", L"z4p_pr0xy", L"sq1m4p_", L"n1kt0_sc4n",
    L"d1rb_bust3r", L"g0bust3r_", L"ff0f_", L"w3b_fuzz3r",
    L"p4r4m_m1n3r", L"4rjun_", L"link_f1nd3r", L"sp1d3r_sc4n",
    L"cr4wl3r_b0t", L"sc4py_pr0", L"tw1st3d_p4ir", L"asy4nc_h4ck",
    L"c0ncu3rr3nt", L"p4r4ll3l_0p", L"thr34d_r4c3", L"d34dl0ck_",
    L"mut3x_l0ck", L"s3m4ph0r3", L"sp1n_l0ck_", L"4t0m1c_0p",
    // More famous reference handles
    L"w0rm_j1m", L"Optik_Fibre", L"Acid_Phreak", L"Scorpion_DG",
    L"Lex_Luthor_LOD", L"Erik_Bloodaxe", L"Knight_Lightning", L"Taran_King",
    L"Pengo_CHAO5", L"Electron_AU", L"Nahshon_Even", L"Ehud_Tenenbaum",
    L"DarkMarket_Admin", L"CarderPlanet_", L"ShadowCrew_", L"Rescator_",
    L"Slavik_JabberZ", L"Tank_WM", L"Boa_Factory", L"Aqua_BotHerd",
    L"Paunch_Exploit", L"J_P_Morgan_", L"Gozi_Coder", L"Silkroad_Mod",
    L"Variety_Jones", L"Chronic_Pain", L"Inigo_SR2", L"DeSnake_Alpha",
    L"OxyMonster_", L"FriendlyChemist", L"Nob_DEA_", L"CircleShifter",
    L"Nom_de_Plume", L"Techno_Viking", L"sn0wden_", L"maning_leaks",
    L"real_assange", L"swartz_legacy", L"geohot_", L"Bunnie_Huang",
    L"HD_Moore_", L"Mudge_L0pht", L"Dildog_", L"Space_Rogue",
    L"Weld_Pond_", L"Kingpin_L0pht", L"Brian_Oblivion", L"Count_Zero_L",
    L"John_Tan_", L"Zatko_", L"Peiter_", L"Solar_Designer",
    L"Fyodor_Nmap", L"Ofir_", L"MrGrey_", L"Corben_",
    L"Cyb3rz3ist_", L"Binary_Jester", L"HexKid_", L"NullChar_",
    L"EscapeCode_", L"SegFault_Sam", L"StackSmash_", L"HeapCorrupt_",
    L"RetAddr_", L"ShellPop_", L"Gadget_Hunt", L"RopNinja_",
    L"SyscallKing_", L"HookMaster_", L"PatchGuard_", L"KernelPanic_",
    L"DriverDev_", L"IoCtlFuzz_", L"PoolCorrupt_", L"TypeConfuse_",
    L"UseAfterFree_", L"DoubleFetch_", L"IntOverflow_", L"FmtString_",
    L"VTable_Hijack", L"VtableSmash_", L"CppVuln_", L"RustSafe_",
    L"GoRoutine_", L"AsyncPwn_", L"PromiseBreak_", L"CallbackHell_",
    L"Proto_Pollute", L"DeserialFlaw", L"PickleRCE_", L"YamlLoad_",
    L"XmlBomb_", L"BillionLaughs", L"XXE_Entity_", L"XPath_Inject",
    L"NoSQL_Inject", L"LDAP_Inject_", L"CRLF_Inject_", L"HeaderSmug",
    L"RequestSmug_", L"H2_Smuggle_", L"CachePwn_", L"CDN_Bypass_",
    L"WAF_Evade_", L"CloudFlare_X", L"AkamaiHunt_", L"EdgeBypass_",
    L"Lambda_Pwn_", L"S3_Bucket_", L"IAM_Privesc_", L"SSRF_Cloud_",
    L"IMDS_Steal_", L"k8s_Escape_", L"ContainerBrk", L"DockerSock_",
    L"PodExec_", L"EtcdDump_", L"HelmChart_", L"TerraformSt",
    L"AnsibleVault", L"JenkinsPwn_", L"GitLabCI_RCE", L"ActionsHack",
    L"SupplyChainX", L"DependConfuse", L"Typosquat_", L"PkgHijack_",
};
extern const int NUM_HACKER_USERNAMES = _countof(g_HackerUsernames);

// ============================================================================
// Intel Market Items - Real breach names with distinct bonuses
// ============================================================================

extern const IntelItemDef g_IntelItems[] = {
    // Data Dumps - real breaches
    { IntelItemType::LinkedInCombo,      L"LinkedIn Combo (700M)",       L"LinkedIn 2021 breach. 700M email:pass combos. Great for credential stuffing.",   300,   0,    L"DATA DUMPS" },
    { IntelItemType::ParkMobileBreach,   L"ParkMobile Breach (21M)",     L"ParkMobile 2021. Email, phone, license plate, hashed passwords.",                400,   0,    L"DATA DUMPS" },
    { IntelItemType::NationalPublicData, L"National Public Data (2.9B)", L"NPD 2024 mega breach. SSN + DOB + address. Most Americans affected.",           2000,  100,  L"DATA DUMPS" },
    { IntelItemType::AutoZoneFullz,      L"AutoZone CC Fullz (185k)",    L"AutoZone 2023 breach. Full CC data w/ CVV, billing addr. 80%+ valid.",          3500,  100,  L"DATA DUMPS" },
    { IntelItemType::MGMResortsDump,     L"MGM Resorts Dump",            L"MGM 2023 breach. Employee + guest data. Corporate emails, internal systems.",    5000,  200,  L"DATA DUMPS" },
    { IntelItemType::TMobileCustomerDB,  L"T-Mobile Customer DB",        L"T-Mobile 2021-2023 breaches. 76M+ records. Phone, IMEI, SIM data.",             1500,  100,  L"DATA DUMPS" },
    { IntelItemType::LastPassVaultData,  L"LastPass Vault Data",         L"LastPass 2022 breach. Encrypted password vaults. Some already cracked.",         8000,  500,  L"DATA DUMPS" },
    { IntelItemType::MOVEitTransferDump, L"MOVEit Transfer Dump",        L"MOVEit 2023. Data from 2,500+ orgs. Government, healthcare, finance.",         10000,  500,  L"DATA DUMPS" },
    { IntelItemType::EquifaxCreditData,  L"Equifax Credit Data (147M)",  L"Equifax 2017. SSN, credit scores, dispute docs. The classic.",                  6000,  200,  L"DATA DUMPS" },
    { IntelItemType::AnthemHealthRecords,L"Anthem Health Records (80M)", L"Anthem 2015. Medical records, SSN, income data. HIPAA goldmine.",               4000,  200,  L"DATA DUMPS" },
    { IntelItemType::FacebookMetaLeak,   L"Facebook/Meta Leak (533M)",   L"Facebook 2021. Phone numbers, emails, locations. OSINT paradise.",               800,   0,    L"DATA DUMPS" },
    { IntelItemType::TwitterXUserData,   L"Twitter/X User Data (200M)",  L"Twitter 2023. Email addresses, follower data, DMs from verified accts.",         600,   0,    L"DATA DUMPS" },
    // Access
    { IntelItemType::CorporateVPNAccess, L"Corporate VPN Credentials",   L"Active VPN access to Fortune 1000. Verified 24h ago. Pulse Secure.",          15000,  500,  L"ACCESS" },
    { IntelItemType::RDPAccess,          L"RDP Access (Admin)",          L"RDP admin access to Windows Server 2019. Clean IP, fresh install.",              8000,  500,  L"ACCESS" },
    // Identity
    { IntelItemType::StolenIdentity,     L"Full Identity Package",       L"Complete identity: SSN, DL scan, utility bills, credit report. Grade A.",      10000,  500,  L"IDENTITY" },
    // Tools
    { IntelItemType::EvilNginxConfig,    L"EvilNginx Phishlet Pack",     L"Pre-configured phishlets for Office365, Gmail, banking portals.",               3000,  100,  L"TOOLS" },
    { IntelItemType::PhishingKit,        L"GoPhish Premium Kit",         L"Responsive phishing pages w/ credential logging. 50+ templates.",               2500,   0,   L"TOOLS" },
    { IntelItemType::ExploitPack,        L"Exploit Collection (2024)",   L"Working exploits for recent CVEs. PoC and weaponized. Includes log4j.",        20000,  500,  L"TOOLS" },
};

// ============================================================================
// IRC Dialogue Template Pools
// ============================================================================

// Success greetings - recruit introduces themselves
extern const wchar_t* g_IRCGreetingsSuccess[] = {
    L"yo, heard you're looking for talent",
    L"sup. got your message on the boards",
    L"hey, saw your posting. i'm interested",
    L"someone vouched for you. what's the gig?",
    L"been lurking your channel. you seem legit",
    L"heard through the grapevine you need hands",
    L"your rep precedes you. let's talk",
    L"got referred by a mutual. what do you need?",
    L"checked your creds. not bad. i'm in if the price is right",
    L"saw your BBS post. you still hiring?",
    L"word on the street is you're building a crew",
    L"been looking for a crew that's actually serious",
    L"my last crew got v& so i'm free. what's up",
    L"heard you don't snitch. that's all i needed to hear",
    L"your opsec checks out. let's do business",
};
extern const int NUM_IRC_GREETINGS_SUCCESS = _countof(g_IRCGreetingsSuccess);

// Scam greetings
extern const wchar_t* g_IRCGreetingsScam[] = {
    L"yo i'm the best in the game. seriously",
    L"trust me bro, i can do anything you need",
    L"hey, heard you got deep pockets. let's work",
    L"i got skills nobody else has. guaranteed",
    L"you need help? i'm your guy. 100%",
    L"someone told me you're paying top dollar",
    L"i can crack anything. just need a small advance",
    L"don't worry about references. i'm too good for that",
};
extern const int NUM_IRC_GREETINGS_SCAM = _countof(g_IRCGreetingsScam);

// Specialty banter per personnel type
extern const SpecialtyLine g_IRCSpecialtyLines[] = {
    // ScriptKiddie
    { PersonnelType::ScriptKiddie,  L"i know my way around metasploit and burp suite" },
    { PersonnelType::ScriptKiddie,  L"mostly webapp stuff. sqli, xss, the basics" },
    { PersonnelType::ScriptKiddie,  L"i'm still learning but i pick things up fast" },
    // Phisher
    { PersonnelType::Phisher,       L"social engineering is my thing. people are the weakest link" },
    { PersonnelType::Phisher,       L"i can clone any login page in under an hour" },
    { PersonnelType::Phisher,       L"got my own gophish setup and evilnginx configs" },
    { PersonnelType::Phisher,       L"spear phishing Fortune 500 employees is my bread and butter" },
    // MalwareCoder
    { PersonnelType::MalwareCoder,  L"i write custom RATs and loaders. fully undetectable" },
    { PersonnelType::MalwareCoder,  L"process injection, unhooking, direct syscalls. the works" },
    { PersonnelType::MalwareCoder,  L"my last implant went 6 months without detection" },
    { PersonnelType::MalwareCoder,  L"c++ and rust. nothing managed, nothing interpreted" },
    // Carder
    { PersonnelType::Carder,        L"i know the carding game inside out. drops, cashout, the whole thing" },
    { PersonnelType::Carder,        L"fullz, high-balance cards, verified bins. i got connects" },
    // NetworkSpecialist
    { PersonnelType::NetworkSpecialist, L"routers, switches, firewalls. cisco, juniper, palo alto" },
    { PersonnelType::NetworkSpecialist, L"i can pivot through any network once i'm in" },
    { PersonnelType::NetworkSpecialist, L"bgp hijacking, vlan hopping, 802.1x bypass" },
    // ExploitDeveloper
    { PersonnelType::ExploitDeveloper, L"heap overflow, use-after-free, type confusion. i find the bugs" },
    { PersonnelType::ExploitDeveloper, L"i've got 3 unreleased CVEs. one of them is a kernel vuln" },
    { PersonnelType::ExploitDeveloper, L"reverse engineering is my life. ida, ghidra, binary ninja" },
    // DataBroker
    { PersonnelType::DataBroker,    L"i've got contacts at every major dump shop" },
    { PersonnelType::DataBroker,    L"data is the new oil and i'm the pipeline" },
    // MoneyLaunderer
    { PersonnelType::MoneyLaunderer, L"crypto tumbling, chain hopping, privacy coins. i clean everything" },
    { PersonnelType::MoneyLaunderer, L"i've moved 7 figures through defi without a trace" },
    // OPSECSpecialist
    { PersonnelType::OPSECSpecialist, L"your opsec has holes. let me fix that before you get caught" },
    { PersonnelType::OPSECSpecialist, L"compartmentalization, air-gapping, plausible deniability" },
    // ZeroDayResearcher
    { PersonnelType::ZeroDayResearcher, L"i sell to brokers. zerodium, crowdfense. six figures per bug" },
    { PersonnelType::ZeroDayResearcher, L"found a chrome v8 bug last month. still unpatched" },
    // InsiderHandler
    { PersonnelType::InsiderHandler, L"i recruit insiders. everyone has a price" },
    { PersonnelType::InsiderHandler, L"got contacts at three banks and a telecom" },
    // InfrastructureArchitect
    { PersonnelType::InfrastructureArchitect, L"bulletproof hosting, tor hidden services, c2 infrastructure" },
    { PersonnelType::InfrastructureArchitect, L"i build infrastructure that survives takedowns" },
};
extern const int NUM_IRC_SPECIALTY_LINES = _countof(g_IRCSpecialtyLines);

// Generic banter anyone might say
extern const wchar_t* g_IRCBanterLines[] = {
    L"what's the pay like?",
    L"how many people you got on the crew?",
    L"i don't work with feds. just putting that out there",
    L"my rates aren't cheap but you get what you pay for",
    L"i've been in the game for years. ask around",
    L"last crew i was with got sloppy. i don't do sloppy",
    L"as long as the opsec is tight, i'm good",
    L"i need guaranteed payment. no scams",
    L"when do we start?",
    L"i can start right now if you need me",
    L"just keep the feds off my back and we're good",
    L"i've got my own tooling. don't need anything from you",
};
extern const int NUM_IRC_BANTER_LINES = _countof(g_IRCBanterLines);

// Success outcome
extern const wchar_t* g_IRCOutcomeSuccess[] = {
    L"alright deal. sending you my pgp key now",
    L"sounds good. i'm in. let's get to work",
    L"perfect. you won't regret this",
    L"deal. i'll be on standby. ping me when you need me",
    L"cool. setting up my workspace now",
    L"great. let's make some money",
    L"done. check your dead drop for my contact info",
    L"locked in. i'll be ready when you are",
};
extern const int NUM_IRC_OUTCOME_SUCCESS = _countof(g_IRCOutcomeSuccess);

// Scam outcome
extern const wchar_t* g_IRCOutcomeScam[] = {
    L"actually... i need you to send the advance payment first",
    L"wait, let me check something real quick...",
    L"haha thanks for the coins. maybe next time use escrow",
    L"you really thought i was legit? lmao",
    L"that wallet address? yeah that's mine now. peace",
    L"pro tip: never trust anyone in a darknet IRC channel",
    L"you got played. it happens to everyone once",
    L"imagine not using escrow in 2024. couldn't be me",
};
extern const int NUM_IRC_OUTCOME_SCAM = _countof(g_IRCOutcomeScam);

// Job unlock intro lines
extern const wchar_t* g_IRCJobUnlockIntros[] = {
    L"yo, saw you grinding on the boards",
    L"heard you've been putting in work lately",
    L"word travels fast in the scene. nice work",
    L"somebody mentioned your name. in a good way",
    L"been watching you. you're ready for something bigger",
    L"your rep is climbing. time to level up",
    L"got something that might interest you",
    L"a contact of mine has a gig. thought of you",
};
extern const int NUM_IRC_JOB_UNLOCK_INTROS = _countof(g_IRCJobUnlockIntros);

// Job unlock closer lines
extern const wchar_t* g_IRCJobUnlockClosers[] = {
    L"check your Jobs panel. good luck out there",
    L"it's in your jobs list now. don't blow it",
    L"go check it out. you didn't hear this from me",
    L"the details are in your panel. stay safe",
    L"i gotta bounce. check your jobs. later",
    L"that's all i got. make it count",
    L"hit me up when you've done a few runs",
    L"peace. and watch your opsec",
};
extern const int NUM_IRC_JOB_UNLOCK_CLOSERS = _countof(g_IRCJobUnlockClosers);
