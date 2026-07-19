#include "../core/game.h"
#include "../data/strings.h"

// ============================================================================
// Complete Job Catalog
// ============================================================================

static JobDef g_JobCatalog[] = {
    // ========================================================================
    // TIER 4 - Script Kiddie Operations (Jobs 0-13)
    // Rep rewards reduced to slow T4 progression
    // ========================================================================
    {
        0, L"Website Defacement", Tier::Four,
        45, 25, 1, 8,  // HIGH heat - feds love catching defacers
        {{SKILL_WEB_APP_SECURITY, 15}, {SKILL_OPSEC, 5}}, 2,
        {}, 0,
        L"Deface small websites with known CMS exploits. HIGH RISK: feds actively monitor for this.",
        false,
        5, 2, DocId::WebSecurityRefs  // prereq: BBS x2
    },
    {
        1, L"Basic Malware Development", Tier::Four,
        90, 75, 1, 3,
        {{SKILL_WINDOWS_EXPLOITATION, 15}, {SKILL_REVERSE_ENGINEERING, 8}}, 2,
        {{SKILL_WINDOWS_EXPLOITATION, 3}}, 1,  // need some WinExploit knowledge
        L"Write simple keyloggers, RATs, and info-stealers using public code. Package with crypters to avoid basic AV.",
        false,
        0, 3, DocId::WinAPIDocs  // prereq: Website Defacement x3
    },
    {
        2, L"Spouse Phone Hacking", Tier::Four,
        60, 150, 1, 1,
        {{SKILL_MOBILE_PLATFORMS, 15}, {SKILL_SOCIAL_ENGINEERING, 8}}, 2,
        {}, 0,
        L"Install spyware on a partner's phone for a jealous client. Physical access makes it trivial.",
        false,
        7, 1, DocId::COUNT
    },
    {
        3, L"Credit Card Skimming", Tier::Four,
        45, 200, 1, 4,
        {{SKILL_FINANCIAL_SYSTEMS, 12}, {SKILL_EMBEDDED_RTOS, 10}}, 2,
        {}, 0,
        L"Attach physical skimmers to ATMs or gas pumps. Harvest mag-stripe data.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        4, L"Bootleg Media Distribution", Tier::Four,
        20, 15, 1, 1,
        {{SKILL_OPSEC, 8}}, 1,
        {}, 0,
        L"Burn pirated movies, music, and software to physical media and sell on the street.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        5, L"BBS Moderation", Tier::Four,
        15, 10, 2, 0,   // keeps 2 rep - community building
        {{SKILL_SOCIAL_ENGINEERING, 8}, {SKILL_OPSEC, 5}}, 2,
        {}, 0,
        L"Moderate an underground bulletin board system. Low pay but builds reputation in the scene.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        6, L"IRC Server Administration", Tier::Four,
        18, 10, 2, 0,   // keeps 2 rep - community building
        {{SKILL_UNIX_LINUX, 10}, {SKILL_NETWORK_ENGINEERING, 8}}, 2,
        {}, 0,
        L"Run and moderate IRC channels used by hackers. Builds connections and street cred.",
        false,
        -1, 0, DocId::UNIXManPages
    },
    {
        7, L"Phishing Friends & Family", Tier::Four,
        35, 50, 1, 2,
        {{SKILL_SOCIAL_ENGINEERING, 15}, {SKILL_WEB_APP_SECURITY, 8}}, 2,
        {{SKILL_SOCIAL_ENGINEERING, 2}}, 1,
        L"Send fake login pages to people in your contacts. Low sophistication, low reward.",
        false,
        5, 5, DocId::COUNT  // prereq: BBS x5 - learning to phish takes time
    },
    {
        8, L"WiFi Cracking", Tier::Four,
        30, 30, 1, 1,
        {{SKILL_NETWORK_ENGINEERING, 15}, {SKILL_OPSEC, 5}}, 2,
        {{SKILL_NETWORK_ENGINEERING, 2}}, 1,
        L"Crack WPA2 handshakes in your neighborhood. Sell access or use for your own operations.",
        false,
        5, 4, DocId::RFCNetworkDocs  // prereq: BBS x4
    },
    {
        9, L"Cryptojacking Scripts", Tier::Four,
        40, 40, 1, 2,
        {{SKILL_WEB_APP_SECURITY, 12}, {SKILL_CRYPTOGRAPHY, 8}}, 2,
        {{SKILL_WEB_APP_SECURITY, 2}}, 1,
        L"Inject cryptocurrency mining scripts into compromised websites. Passive income from visitors' CPUs.",
        false,
        0, 3, DocId::WebSecurityRefs  // prereq: Website Defacement x3
    },
    {
        10, L"Game Account Theft", Tier::Four,
        30, 35, 1, 1,
        {{SKILL_SOCIAL_ENGINEERING, 12}, {SKILL_DATABASE_SYSTEMS, 8}}, 2,
        {{SKILL_SOCIAL_ENGINEERING, 2}}, 1,
        L"Steal and resell gaming accounts. Credential stuffing with leaked combo lists.",
        false,
        7, 3, DocId::COUNT  // prereq: Phishing x3
    },
    {
        11, L"Social Media Hijacking", Tier::Four,
        45, 60, 1, 2,
        {{SKILL_SOCIAL_ENGINEERING, 15}, {SKILL_WEB_APP_SECURITY, 8}}, 2,
        {{SKILL_SOCIAL_ENGINEERING, 3}}, 1,
        L"Take over Instagram, Twitter, TikTok accounts. Resell OG usernames or ransom back to owners.",
        false,
        10, 3, DocId::COUNT  // prereq: Game Account Theft x3
    },
    {
        12, L"Fake Review Services", Tier::Four,
        18, 20, 1, 1,
        {{SKILL_SOCIAL_ENGINEERING, 8}}, 1,
        {}, 0,
        L"Post fake reviews on Amazon, Yelp, Google. Sell review boosting to shady businesses.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        13, L"DDoS Minecraft Server", Tier::Four,
        30, 45, 1, 15,  // MASSIVE heat - feds take DDoS very seriously
        {{SKILL_NETWORK_ENGINEERING, 12}, {SKILL_UNIX_LINUX, 8}}, 2,
        {}, 0,
        L"Take down a kid's Minecraft server for $45. Feds treat DDoS like a federal crime. Because it is.",
        false,
        8, 2, DocId::RFCNetworkDocs  // prereq: WiFi Cracking x2
    },

    // ========================================================================
    // TIER 3 - Intermediate Cybercriminal (Jobs 14-30)
    // ========================================================================
    {
        14, L"Private Torrent Tracker", Tier::Three,
        180, 300, 4, 3,
        {{SKILL_UNIX_LINUX, 3}, {SKILL_NETWORK_ENGINEERING, 2}}, 2,
        {{SKILL_UNIX_LINUX, 5}}, 1,
        L"Run an invite-only tracker for pirated content. Manage ratios, seeds, and a community.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        15, L"Intermediate Malware Dev", Tier::Three,
        300, 800, 5, 5,
        {{SKILL_WINDOWS_EXPLOITATION, 4}, {SKILL_REVERSE_ENGINEERING, 3}}, 2,
        {{SKILL_REVERSE_ENGINEERING, 5}}, 1,
        L"Develop polymorphic malware with evasion capabilities. Custom C2, process injection, persistence.",
        false,
        -1, 0, DocId::WinAPIDocs
    },
    {
        16, L"Basic Exploit Development", Tier::Three,
        360, 1200, 6, 4,
        {{SKILL_REVERSE_ENGINEERING, 4}, {SKILL_WINDOWS_EXPLOITATION, 3}}, 2,
        {{SKILL_REVERSE_ENGINEERING, 8}}, 1,
        L"Write exploits for known CVEs. Buffer overflows, use-after-free. Memory corruption basics.",
        false,
        -1, 0, DocId::ReverseEngineeringManuals
    },
    {
        17, L"Crypto Wallet Phishing", Tier::Three,
        120, 2000, 4, 5,
        {{SKILL_SOCIAL_ENGINEERING, 3}, {SKILL_CRYPTOGRAPHY, 2}, {SKILL_WEB_APP_SECURITY, 2}}, 3,
        {{SKILL_SOCIAL_ENGINEERING, 8}, {SKILL_CRYPTOGRAPHY, 3}}, 2,
        L"Create convincing fake wallet interfaces. Drain crypto wallets through approval phishing.",
        false,
        -1, 0, DocId::CryptoProtocolDocs
    },
    {
        18, L"Small Data Dump Sales", Tier::Three,
        150, 500, 3, 4,
        {{SKILL_DATABASE_SYSTEMS, 3}, {SKILL_OPSEC, 2}}, 2,
        {{SKILL_DATABASE_SYSTEMS, 5}}, 1,
        L"Sell breached databases (10k-100k records) on underground forums. Email/password combos, basic PII.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        19, L"Credit Card Data Sales", Tier::Three,
        200, 1500, 5, 6,
        {{SKILL_FINANCIAL_SYSTEMS, 3}, {SKILL_DATABASE_SYSTEMS, 2}, {SKILL_OPSEC, 2}}, 3,
        {{SKILL_FINANCIAL_SYSTEMS, 5}}, 1,
        L"Sell CC fullz in bulk. Quality cards with high limits command premium prices.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        20, L"Blackmail-as-a-Service", Tier::Three,
        240, 3000, 4, 7,
        {{SKILL_SOCIAL_ENGINEERING, 4}, {SKILL_OPSEC, 2}}, 2,
        {{SKILL_SOCIAL_ENGINEERING, 10}}, 1,
        L"Leverage compromised personal data to extort targets. Sextortion, business secrets.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        21, L"BBS ANSI Art Creation", Tier::Three,
        90, 100, 3, 0,
        {{SKILL_ART, 5}}, 1,
        {}, 0,
        L"Create ANSI/ASCII art for underground BBS scenes. Massive cred in the community.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        22, L"SIM Swapping", Tier::Three,
        180, 2500, 5, 6,
        {{SKILL_TELECOMMUNICATIONS, 4}, {SKILL_SOCIAL_ENGINEERING, 3}}, 2,
        {{SKILL_SOCIAL_ENGINEERING, 8}, {SKILL_TELECOMMUNICATIONS, 3}}, 2,
        L"Bribe or social-engineer mobile carrier employees to port victim's phone number. Bypass 2FA.",
        false,
        -1, 0, DocId::SocialEngineeringPlaybook
    },
    {
        23, L"Fake ID Production", Tier::Three,
        200, 400, 3, 5,
        {{SKILL_ART, 4}, {SKILL_OPSEC, 2}}, 2,
        {}, 0,
        L"Create convincing fake driver's licenses and state IDs using templates and professional printers.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        24, L"Carding - Online Purchases", Tier::Three,
        100, 600, 3, 5,
        {{SKILL_FINANCIAL_SYSTEMS, 3}, {SKILL_OPSEC, 2}}, 2,
        {{SKILL_FINANCIAL_SYSTEMS, 5}}, 1,
        L"Use stolen credit card data to purchase goods online, ship to drops, resell.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        25, L"Credential Stuffing Service", Tier::Three,
        160, 700, 4, 4,
        {{SKILL_DATABASE_SYSTEMS, 3}, {SKILL_WEB_APP_SECURITY, 2}}, 2,
        {{SKILL_DATABASE_SYSTEMS, 8}}, 1,
        L"Automated testing of leaked username/password combos against hundreds of sites.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        26, L"Darknet Drug Market Vendor", Tier::Three,
        250, 2000, 5, 7,
        {{SKILL_OPSEC, 4}, {SKILL_CRYPTOGRAPHY, 2}}, 2,
        {{SKILL_OPSEC, 8}}, 1,
        L"Source and sell narcotics on darknet markets. Dead drops, PGP encryption, escrow.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        27, L"Money Mule Recruitment", Tier::Three,
        180, 800, 4, 5,
        {{SKILL_SOCIAL_ENGINEERING, 3}, {SKILL_FINANCIAL_SYSTEMS, 2}}, 2,
        {{SKILL_SOCIAL_ENGINEERING, 8}}, 1,
        L"Recruit people to receive and forward stolen funds. Build a network of unwitting accomplices.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        28, L"Tech Support Scam Op", Tier::Three,
        130, 500, 3, 4,
        {{SKILL_SOCIAL_ENGINEERING, 4}, {SKILL_WINDOWS_EXPLOITATION, 1}}, 2,
        {{SKILL_SOCIAL_ENGINEERING, 6}}, 1,
        L"Run fake tech support centers targeting elderly victims. Pop up fake virus warnings.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        29, L"SEO Poisoning / Malvertising", Tier::Three,
        140, 450, 3, 3,
        {{SKILL_WEB_APP_SECURITY, 3}, {SKILL_SOCIAL_ENGINEERING, 2}}, 2,
        {{SKILL_WEB_APP_SECURITY, 8}}, 1,
        L"Poison search engine results to drive traffic to malware-laden pages.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        30, L"Spam Botnet Operation", Tier::Three,
        200, 350, 4, 5,
        {{SKILL_NETWORK_ENGINEERING, 3}, {SKILL_UNIX_LINUX, 2}}, 2,
        {{SKILL_NETWORK_ENGINEERING, 8}, {SKILL_UNIX_LINUX, 5}}, 2,
        L"Operate botnets to send millions of spam emails. Sell access to other criminals.",
        false,
        -1, 0, DocId::RFCNetworkDocs
    },

    // ========================================================================
    // TIER 2 - Professional Threat Actor (Jobs 31-49)
    // ========================================================================
    {
        31, L"Corporate Phishing Campaign", Tier::Two,
        600, 15000, 10, 8,
        {{SKILL_SOCIAL_ENGINEERING, 5}, {SKILL_WEB_APP_SECURITY, 3}}, 2,
        {{SKILL_SOCIAL_ENGINEERING, 20}, {SKILL_WEB_APP_SECURITY, 15}}, 2,
        L"Spear-phishing campaigns targeting corporate employees. Custom lures, lookalike domains.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        32, L"Router/Network Hacking", Tier::Two,
        480, 8000, 8, 7,
        {{SKILL_NETWORK_ENGINEERING, 5}, {SKILL_UNIX_LINUX, 3}}, 2,
        {{SKILL_NETWORK_ENGINEERING, 20}}, 1,
        L"Exploit vulnerabilities in Cisco, Juniper, MikroTik routers. Sell persistent access.",
        false,
        -1, 0, DocId::RFCNetworkDocs
    },
    {
        33, L"Advanced Malware Development", Tier::Two,
        900, 25000, 12, 8,
        {{SKILL_WINDOWS_EXPLOITATION, 5}, {SKILL_REVERSE_ENGINEERING, 5}, {SKILL_UNIX_LINUX, 2}}, 3,
        {{SKILL_REVERSE_ENGINEERING, 25}, {SKILL_WINDOWS_EXPLOITATION, 20}}, 2,
        L"Develop rootkits, bootkits, fileless malware. EDR evasion, living-off-the-land techniques.",
        false,
        -1, 0, DocId::WinAPIDocs
    },
    {
        34, L"Advanced Exploit Development", Tier::Two,
        1200, 40000, 15, 7,
        {{SKILL_REVERSE_ENGINEERING, 6}, {SKILL_WINDOWS_EXPLOITATION, 4}}, 2,
        {{SKILL_REVERSE_ENGINEERING, 35}}, 1,
        L"Develop reliable exploits for recent CVEs. Heap spraying, ROP chains, kernel exploits.",
        false,
        -1, 0, DocId::ReverseEngineeringManuals
    },
    {
        35, L"Hardware Skimmer Manufacturing", Tier::Two,
        540, 12000, 8, 8,
        {{SKILL_EMBEDDED_RTOS, 5}, {SKILL_FINANCIAL_SYSTEMS, 3}, {SKILL_RADIO_FREQUENCY, 3}}, 3,
        {{SKILL_EMBEDDED_RTOS, 15}, {SKILL_FINANCIAL_SYSTEMS, 10}}, 2,
        L"Design and manufacture advanced card skimmers with Bluetooth exfiltration.",
        false,
        -1, 0, DocId::EmbeddedSystemsReference
    },
    {
        36, L"Large Data Dump Brokerage", Tier::Two,
        720, 50000, 12, 9,
        {{SKILL_DATABASE_SYSTEMS, 5}, {SKILL_OPSEC, 4}, {SKILL_SOCIAL_ENGINEERING, 2}}, 3,
        {{SKILL_DATABASE_SYSTEMS, 25}, {SKILL_OPSEC, 15}}, 2,
        L"Broker multi-million record data breaches. Healthcare, financial, corporate databases.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        37, L"Ransomware-as-a-Service", Tier::Two,
        1080, 75000, 15, 10,
        {{SKILL_WINDOWS_EXPLOITATION, 5}, {SKILL_CRYPTOGRAPHY, 5}, {SKILL_NETWORK_ENGINEERING, 3}}, 3,
        {{SKILL_CRYPTOGRAPHY, 20}, {SKILL_WINDOWS_EXPLOITATION, 25}}, 2,
        L"Develop and operate ransomware with affiliate programs. Encryption, payment portal, negotiation.",
        false,
        -1, 0, DocId::WinAPIDocs
    },
    {
        38, L"Advanced ANSI/BBS Art", Tier::Two,
        300, 500, 8, 0,
        {{SKILL_ART, 8}}, 1,
        {{SKILL_ART, 15}}, 1,
        L"Elite-level ANSI artwork for the scene. You're now a legend in underground art groups.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        39, L"Unattributable Infrastructure", Tier::Two,
        600, 5000, 10, -5,  // Reduces heat!
        {{SKILL_OPSEC, 6}, {SKILL_NETWORK_ENGINEERING, 3}, {SKILL_UNIX_LINUX, 3}}, 3,
        {{SKILL_OPSEC, 20}, {SKILL_NETWORK_ENGINEERING, 15}}, 2,
        L"Set up layered VPN chains, Tor hidden services, bulletproof VPS chains. The backbone of staying free.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        40, L"Business Email Compromise", Tier::Two,
        480, 35000, 10, 8,
        {{SKILL_SOCIAL_ENGINEERING, 5}, {SKILL_FINANCIAL_SYSTEMS, 3}}, 2,
        {{SKILL_SOCIAL_ENGINEERING, 25}, {SKILL_FINANCIAL_SYSTEMS, 15}}, 2,
        L"Impersonate executives to redirect wire transfers. Average real-world BEC nets $130k.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        41, L"ATM Jackpotting", Tier::Two,
        540, 20000, 10, 9,
        {{SKILL_EMBEDDED_RTOS, 5}, {SKILL_FINANCIAL_SYSTEMS, 4}}, 2,
        {{SKILL_EMBEDDED_RTOS, 20}, {SKILL_FINANCIAL_SYSTEMS, 15}}, 2,
        L"Deploy malware to ATMs to force cash dispensing. Requires physical access and technical expertise.",
        false,
        -1, 0, DocId::EmbeddedSystemsReference
    },
    {
        42, L"Fake Passport Production", Tier::Two,
        600, 5000, 8, 8,
        {{SKILL_ART, 5}, {SKILL_OPSEC, 3}}, 2,
        {{SKILL_ART, 10}}, 1,
        L"Produce high-quality counterfeit passports using stolen blanks. Biometric chip cloning.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        43, L"Darknet Market Administration", Tier::Two,
        900, 30000, 15, 9,
        {{SKILL_OPSEC, 5}, {SKILL_CRYPTOGRAPHY, 4}, {SKILL_UNIX_LINUX, 4}}, 3,
        {{SKILL_OPSEC, 25}, {SKILL_UNIX_LINUX, 20}, {SKILL_CRYPTOGRAPHY, 15}}, 3,
        L"Run your own darknet marketplace. Manage vendors, escrow, dispute resolution. Requires IRC admin experience.",
        false,
        6, 10, DocId::COUNT  // prereq: IRC Server Administration x10
    },
    {
        44, L"Botnet Rental Service", Tier::Two,
        720, 18000, 10, 8,
        {{SKILL_NETWORK_ENGINEERING, 5}, {SKILL_UNIX_LINUX, 4}}, 2,
        {{SKILL_NETWORK_ENGINEERING, 25}, {SKILL_UNIX_LINUX, 20}}, 2,
        L"Operate large botnets (100k+ nodes). Rent out for DDoS, spam, credential stuffing, crypto mining.",
        false,
        -1, 0, DocId::RFCNetworkDocs
    },
    {
        45, L"Counterfeit Goods E-Commerce", Tier::Two,
        400, 8000, 6, 5,
        {{SKILL_WEB_APP_SECURITY, 2}, {SKILL_SOCIAL_ENGINEERING, 3}, {SKILL_OPSEC, 2}}, 3,
        {{SKILL_WEB_APP_SECURITY, 10}, {SKILL_OPSEC, 10}}, 2,
        L"Run online storefronts selling counterfeit luxury goods, electronics, pharmaceuticals.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        46, L"Wire Fraud Network", Tier::Two,
        600, 25000, 10, 9,
        {{SKILL_FINANCIAL_SYSTEMS, 5}, {SKILL_SOCIAL_ENGINEERING, 4}}, 2,
        {{SKILL_FINANCIAL_SYSTEMS, 20}, {SKILL_SOCIAL_ENGINEERING, 20}}, 2,
        L"Orchestrate complex wire fraud schemes. Romance scams, investment fraud at scale.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        47, L"Underground Pharmacy Op", Tier::Two,
        500, 15000, 8, 7,
        {{SKILL_OPSEC, 4}, {SKILL_CRYPTOGRAPHY, 2}}, 2,
        {{SKILL_OPSEC, 20}}, 1,
        L"Sell prescription drugs and controlled substances via darknet. Manage supply chains.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        48, L"RFID Cloning & Access", Tier::Two,
        360, 6000, 7, 6,
        {{SKILL_RADIO_FREQUENCY, 5}, {SKILL_EMBEDDED_RTOS, 3}}, 2,
        {{SKILL_RADIO_FREQUENCY, 10}, {SKILL_EMBEDDED_RTOS, 10}}, 2,
        L"Clone building access cards, car key fobs, NFC payment cards using Proxmark.",
        false,
        -1, 0, DocId::EmbeddedSystemsReference
    },
    {
        49, L"Crypto Mixing/Tumbling", Tier::Two,
        480, 10000, 8, 6,
        {{SKILL_CRYPTOGRAPHY, 5}, {SKILL_FINANCIAL_SYSTEMS, 3}}, 2,
        {{SKILL_CRYPTOGRAPHY, 15}, {SKILL_FINANCIAL_SYSTEMS, 10}}, 2,
        L"Operate a crypto tumbling service to launder funds. CoinJoin, chain-hopping, DeFi mixing.",
        false,
        -1, 0, DocId::COUNT
    },

    // ========================================================================
    // TIER 1 - Elite / Nation-State Tier (Jobs 50-65)
    // ========================================================================
    {
        50, L"Gov Network Access Sales", Tier::One,
        3600, 500000, 30, 12,
        {{SKILL_UNIX_LINUX, 6}, {SKILL_NETWORK_ENGINEERING, 5}, {SKILL_OPSEC, 5}}, 3,
        {{SKILL_UNIX_LINUX, 40}, {SKILL_NETWORK_ENGINEERING, 40}, {SKILL_OPSEC, 35}}, 3,
        L"Sell persistent access to government networks (.gov/.mil). Initial access broker for nation-states.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        51, L"ISP Infrastructure Access", Tier::One,
        2700, 250000, 25, 10,
        {{SKILL_NETWORK_ENGINEERING, 6}, {SKILL_TELECOMMUNICATIONS, 5}}, 2,
        {{SKILL_NETWORK_ENGINEERING, 45}, {SKILL_TELECOMMUNICATIONS, 30}}, 2,
        L"Compromise and sell access to ISP backbone infrastructure. Enable mass surveillance.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        52, L"Enterprise Credential Theft", Tier::One,
        1800, 100000, 20, 9,
        {{SKILL_SOCIAL_ENGINEERING, 5}, {SKILL_DATABASE_SYSTEMS, 5}, {SKILL_WINDOWS_EXPLOITATION, 4}}, 3,
        {{SKILL_SOCIAL_ENGINEERING, 35}, {SKILL_DATABASE_SYSTEMS, 30}, {SKILL_WINDOWS_EXPLOITATION, 30}}, 3,
        L"Steal domain admin credentials at Fortune 500 companies. Golden ticket attacks, Kerberoasting.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        53, L"Nation-State Malware Dev", Tier::One,
        7200, 1000000, 40, 10,
        {{SKILL_WINDOWS_EXPLOITATION, 7}, {SKILL_REVERSE_ENGINEERING, 7}, {SKILL_UNIX_LINUX, 5}}, 3,
        {{SKILL_REVERSE_ENGINEERING, 50}, {SKILL_WINDOWS_EXPLOITATION, 45}, {SKILL_UNIX_LINUX, 35}}, 3,
        L"Develop Stuxnet-tier malware. SCADA-targeting, air-gap-jumping, multi-stage payloads.",
        false,
        -1, 0, DocId::WinAPIDocs
    },
    {
        54, L"Zero-Day Development", Tier::One,
        5400, 750000, 35, 8,
        {{SKILL_REVERSE_ENGINEERING, 8}, {SKILL_WINDOWS_EXPLOITATION, 6}}, 2,
        {{SKILL_REVERSE_ENGINEERING, 60}}, 1,
        L"Discover and weaponize unknown vulnerabilities. Browser zero-days. The pinnacle of offensive security.",
        false,
        -1, 0, DocId::ReverseEngineeringManuals
    },
    {
        55, L"Crypto Laundering at Scale", Tier::One,
        3600, 200000, 20, 10,
        {{SKILL_CRYPTOGRAPHY, 6}, {SKILL_FINANCIAL_SYSTEMS, 6}}, 2,
        {{SKILL_CRYPTOGRAPHY, 35}, {SKILL_FINANCIAL_SYSTEMS, 30}}, 2,
        L"Launder millions through DeFi protocols, cross-chain bridges, privacy coins.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        56, L"Full Identity Removal", Tier::One,
        2700, 150000, 20, -10,  // Reduces heat!
        {{SKILL_OPSEC, 8}, {SKILL_DATABASE_SYSTEMS, 5}, {SKILL_SOCIAL_ENGINEERING, 4}}, 3,
        {{SKILL_OPSEC, 40}, {SKILL_DATABASE_SYSTEMS, 30}}, 2,
        L"Erase someone's digital footprint completely. Scrub databases, delete records, create new identities.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        57, L"Classified Info Brokerage", Tier::One,
        5400, 2000000, 50, 15,
        {{SKILL_OPSEC, 6}, {SKILL_SOCIAL_ENGINEERING, 6}}, 2,
        {{SKILL_OPSEC, 45}, {SKILL_SOCIAL_ENGINEERING, 40}}, 2,
        L"Broker stolen classified documents between nation-states and intelligence agencies.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        58, L"Bulletproof Hosting Service", Tier::One,
        1800, 80000, 15, 5,
        {{SKILL_UNIX_LINUX, 6}, {SKILL_NETWORK_ENGINEERING, 5}, {SKILL_OPSEC, 5}}, 3,
        {{SKILL_UNIX_LINUX, 40}, {SKILL_NETWORK_ENGINEERING, 35}, {SKILL_OPSEC, 30}}, 3,
        L"Operate hosting infrastructure immune to takedowns. Host C2 servers, dark markets, leak sites.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        59, L"Supply Chain Attack Dev", Tier::One,
        4800, 800000, 35, 12,
        {{SKILL_REVERSE_ENGINEERING, 7}, {SKILL_WINDOWS_EXPLOITATION, 5}, {SKILL_UNIX_LINUX, 5}}, 3,
        {{SKILL_REVERSE_ENGINEERING, 50}, {SKILL_WINDOWS_EXPLOITATION, 40}, {SKILL_UNIX_LINUX, 35}}, 3,
        L"Compromise software build pipelines to distribute malware through legitimate updates.",
        false,
        -1, 0, DocId::ReverseEngineeringManuals
    },
    {
        60, L"Election Interference", Tier::One,
        5400, 1500000, 45, 15,
        {{SKILL_SOCIAL_ENGINEERING, 7}, {SKILL_WEB_APP_SECURITY, 5}, {SKILL_OPSEC, 5}}, 3,
        {{SKILL_SOCIAL_ENGINEERING, 45}, {SKILL_OPSEC, 40}, {SKILL_WEB_APP_SECURITY, 30}}, 3,
        L"Conduct influence operations: hack-and-leak, disinformation campaigns. Nation-state contract work.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        61, L"Critical Infrastructure Attack", Tier::One,
        7200, 2000000, 50, 15,
        {{SKILL_EMBEDDED_RTOS, 8}, {SKILL_NETWORK_ENGINEERING, 5}, {SKILL_UNIX_LINUX, 5}}, 3,
        {{SKILL_EMBEDDED_RTOS, 45}, {SKILL_NETWORK_ENGINEERING, 40}}, 2,
        L"Target power grids, water treatment, transportation. SCADA/ICS exploitation.",
        false,
        -1, 0, DocId::EmbeddedSystemsReference
    },
    {
        62, L"Submarine Cable Interception", Tier::One,
        5400, 1200000, 40, 12,
        {{SKILL_TELECOMMUNICATIONS, 8}, {SKILL_RADIO_FREQUENCY, 5}, {SKILL_NETWORK_ENGINEERING, 5}}, 3,
        {{SKILL_TELECOMMUNICATIONS, 40}, {SKILL_RADIO_FREQUENCY, 30}, {SKILL_NETWORK_ENGINEERING, 40}}, 3,
        L"Tap undersea cables or intercept satellite communications. Signals intelligence at the highest level.",
        false,
        -1, 0, DocId::RFCNetworkDocs
    },
    {
        63, L"Deepfake-as-a-Service", Tier::One,
        2400, 100000, 15, 8,
        {{SKILL_SOCIAL_ENGINEERING, 5}, {SKILL_AI_ML, 5}}, 2,
        {{SKILL_SOCIAL_ENGINEERING, 30}}, 1,
        L"Generate convincing deepfake video/audio for high-value targets. CEO fraud, political manipulation.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        64, L"Weapons Trafficking", Tier::One,
        3600, 300000, 25, 14,
        {{SKILL_OPSEC, 6}, {SKILL_CRYPTOGRAPHY, 4}}, 2,
        {{SKILL_OPSEC, 40}, {SKILL_CRYPTOGRAPHY, 25}}, 2,
        L"Facilitate online arms deals through encrypted channels. Connect buyers and sellers.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        65, L"Insider Threat Recruitment", Tier::One,
        2700, 200000, 20, 10,
        {{SKILL_SOCIAL_ENGINEERING, 7}}, 1,
        {{SKILL_SOCIAL_ENGINEERING, 40}}, 1,
        L"Recruit insiders at target organizations. Bribe, coerce, or ideologically motivate employees.",
        false,
        -1, 0, DocId::COUNT
    },

    // ========================================================================
    // PRESTIGE - Legitimate Company Jobs (Jobs 66-85)
    // ========================================================================

    // Legitimate Tier 4 (Startup Phase)
    {
        66, L"Vulnerability Assessment", Tier::Four,
        120, 35, 1, 0,
        {{SKILL_WEB_APP_SECURITY, 2}, {SKILL_NETWORK_ENGINEERING, 1}}, 2,
        {}, 0,
        L"Scan client websites for known vulnerabilities. Basic automated scanning.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        67, L"Security Awareness Training", Tier::Four,
        90, 50, 1, 0,
        {{SKILL_SOCIAL_ENGINEERING, 2}}, 1,
        {}, 0,
        L"Train employees not to click phishing links. The most basic defense.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        68, L"Penetration Testing (Basic)", Tier::Four,
        180, 200, 2, 0,
        {{SKILL_WEB_APP_SECURITY, 2}, {SKILL_NETWORK_ENGINEERING, 2}}, 2,
        {}, 0,
        L"Test client network with standard tools. Nmap, Burp Suite, Metasploit basics.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        69, L"Malware Analysis (Basic)", Tier::Four,
        150, 150, 2, 0,
        {{SKILL_REVERSE_ENGINEERING, 2}, {SKILL_WINDOWS_EXPLOITATION, 1}}, 2,
        {}, 0,
        L"Analyze malware samples for AV companies. Behavioral analysis in sandboxes.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        70, L"Digital Forensics (Small)", Tier::Four,
        200, 300, 2, 0,
        {{SKILL_FORENSIC_EVASION, 3}, {SKILL_DATABASE_SYSTEMS, 1}}, 2,
        {}, 0,
        L"Investigate minor incidents. Disk imaging, log analysis, timeline reconstruction.",
        true,
        -1, 0, DocId::COUNT
    },

    // Legitimate Tier 3 (Growing Firm)
    {
        71, L"Bug Bounty Hunting", Tier::Three,
        360, 2000, 4, 0,
        {{SKILL_WEB_APP_SECURITY, 4}, {SKILL_REVERSE_ENGINEERING, 2}}, 2,
        {{SKILL_WEB_APP_SECURITY, 10}}, 1,
        L"Find and report vulnerabilities for bounties. HackerOne, Bugcrowd programs.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        72, L"Incident Response", Tier::Three,
        480, 5000, 5, 0,
        {{SKILL_FORENSIC_EVASION, 4}, {SKILL_NETWORK_ENGINEERING, 3}}, 2,
        {{SKILL_FORENSIC_EVASION, 8}}, 1,
        L"Respond to active breaches at client organizations. Containment, eradication, recovery.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        73, L"Threat Intelligence Reports", Tier::Three,
        300, 3000, 4, 0,
        {{SKILL_OPSEC, 3}, {SKILL_SOCIAL_ENGINEERING, 2}}, 2,
        {{SKILL_OPSEC, 8}}, 1,
        L"Produce intel reports on APT groups. Track TTPs, IOCs, and attribution.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        74, L"Red Team Engagement", Tier::Three,
        600, 8000, 6, 0,
        {{SKILL_SOCIAL_ENGINEERING, 4}, {SKILL_WINDOWS_EXPLOITATION, 3}, {SKILL_NETWORK_ENGINEERING, 3}}, 3,
        {{SKILL_SOCIAL_ENGINEERING, 12}, {SKILL_WINDOWS_EXPLOITATION, 10}}, 2,
        L"Full-scope offensive testing with physical, social, and cyber vectors.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        75, L"Compliance Auditing", Tier::Three,
        400, 4000, 3, 0,
        {{SKILL_DATABASE_SYSTEMS, 2}, {SKILL_NETWORK_ENGINEERING, 2}}, 2,
        {{SKILL_DATABASE_SYSTEMS, 5}}, 1,
        L"SOC2, PCI-DSS, HIPAA compliance audits. Checkbox security pays well.",
        true,
        -1, 0, DocId::COUNT
    },

    // Legitimate Tier 2 (Established Firm)
    {
        76, L"Enterprise Penetration Test", Tier::Two,
        900, 25000, 10, 0,
        {{SKILL_NETWORK_ENGINEERING, 5}, {SKILL_WINDOWS_EXPLOITATION, 4}, {SKILL_WEB_APP_SECURITY, 3}}, 3,
        {{SKILL_NETWORK_ENGINEERING, 20}, {SKILL_WINDOWS_EXPLOITATION, 15}}, 2,
        L"Complex network pentests for large organizations. Active Directory, cloud, hybrid.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        77, L"Malware Reverse Engineering", Tier::Two,
        720, 15000, 8, 0,
        {{SKILL_REVERSE_ENGINEERING, 6}, {SKILL_WINDOWS_EXPLOITATION, 3}}, 2,
        {{SKILL_REVERSE_ENGINEERING, 25}}, 1,
        L"Deep malware analysis for law enforcement. Unpack, deobfuscate, extract C2 infrastructure.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        78, L"Custom Security Tool Dev", Tier::Two,
        1080, 20000, 10, 0,
        {{SKILL_REVERSE_ENGINEERING, 4}, {SKILL_UNIX_LINUX, 4}, {SKILL_WINDOWS_EXPLOITATION, 3}}, 3,
        {{SKILL_REVERSE_ENGINEERING, 20}, {SKILL_UNIX_LINUX, 15}}, 2,
        L"Build custom security tools for clients. EDR solutions, SIEM integrations.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        79, L"DFIR Major Incident", Tier::Two,
        1200, 40000, 12, 0,
        {{SKILL_FORENSIC_EVASION, 6}, {SKILL_NETWORK_ENGINEERING, 4}, {SKILL_DATABASE_SYSTEMS, 3}}, 3,
        {{SKILL_FORENSIC_EVASION, 20}, {SKILL_NETWORK_ENGINEERING, 15}}, 2,
        L"Lead incident response at Fortune 500. Breach scope, attribution, remediation.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        80, L"Security Architecture Review", Tier::Two,
        900, 30000, 10, 0,
        {{SKILL_NETWORK_ENGINEERING, 5}, {SKILL_OPSEC, 4}, {SKILL_UNIX_LINUX, 3}}, 3,
        {{SKILL_NETWORK_ENGINEERING, 20}, {SKILL_OPSEC, 15}}, 2,
        L"Design secure infrastructure for enterprises. Zero-trust, defense-in-depth.",
        true,
        -1, 0, DocId::COUNT
    },

    // Legitimate Tier 1 (Elite Consultancy / Government Contractor)
    {
        81, L"Government Security Contract", Tier::One,
        3600, 200000, 25, 0,
        {{SKILL_OPSEC, 6}, {SKILL_NETWORK_ENGINEERING, 5}, {SKILL_UNIX_LINUX, 4}}, 3,
        {{SKILL_OPSEC, 35}, {SKILL_NETWORK_ENGINEERING, 30}}, 2,
        L"Classified work for defense agencies. Top-secret clearance required.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        82, L"Zero-Day Disclosure", Tier::One,
        5400, 500000, 35, 0,
        {{SKILL_REVERSE_ENGINEERING, 8}, {SKILL_WINDOWS_EXPLOITATION, 5}}, 2,
        {{SKILL_REVERSE_ENGINEERING, 55}}, 1,
        L"Find and responsibly disclose zero-days. The legitimate path to the same pinnacle.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        83, L"Nation-State Attribution", Tier::One,
        4800, 300000, 30, 0,
        {{SKILL_FORENSIC_EVASION, 7}, {SKILL_OPSEC, 5}, {SKILL_NETWORK_ENGINEERING, 4}}, 3,
        {{SKILL_FORENSIC_EVASION, 35}, {SKILL_OPSEC, 30}}, 2,
        L"Attribute attacks to specific APT groups. The detective work of cyberwar.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        84, L"Critical Infrastructure Protection", Tier::One,
        5400, 400000, 30, 0,
        {{SKILL_EMBEDDED_RTOS, 7}, {SKILL_NETWORK_ENGINEERING, 5}, {SKILL_OPSEC, 4}}, 3,
        {{SKILL_EMBEDDED_RTOS, 35}, {SKILL_NETWORK_ENGINEERING, 30}}, 2,
        L"Protect power grids, water systems. The same skills, defending instead of attacking.",
        true,
        -1, 0, DocId::COUNT
    },
    {
        85, L"Cyber Policy Advisory", Tier::One,
        3600, 250000, 25, 0,
        {{SKILL_SOCIAL_ENGINEERING, 5}, {SKILL_OPSEC, 5}}, 2,
        {{SKILL_SOCIAL_ENGINEERING, 30}, {SKILL_OPSEC, 30}}, 2,
        L"Advise governments on cyber policy. Shape the rules of the digital battlefield.",
        true,
        -1, 0, DocId::COUNT
    },

    // ========================================================================
    // SPECIAL - Forum Tip Follow-up (Job 86)
    // Only available when pendingForumTip is true
    // ========================================================================
    {
        86, L"Follow Up on Forum Tip", Tier::Four,
        20, 0, 2, 0,  // payout set dynamically, 2 rep for posting about it
        {{SKILL_SOCIAL_ENGINEERING, 1}, {SKILL_OPSEC, 1}}, 2,
        {}, 0,
        L"A contact shared a lead on the forums. Do the legwork and post your results for rep.",
        false,
        -1, 0, DocId::COUNT
    },

    // ========================================================================
    // GAME HACKING JOBS (Jobs 87-94)
    // Train Windows Exploitation + Reverse Engineering heavily
    // ========================================================================

    // Tier 4 Game Hacking (0 heat - risk is bans, not feds)
    {
        87, L"Cheat Table Sales", Tier::Four,
        35, 25, 1, 0,  // 0 heat - game hacking doesn't attract feds
        {{SKILL_WINDOWS_EXPLOITATION, 15}, {SKILL_REVERSE_ENGINEERING, 8}}, 2,
        {}, 0,
        L"Build and sell Cheat Engine tables on forums. Find memory addresses, AOB scans, pointer chains.",
        false,
        5, 3, DocId::COUNT  // prereq: BBS x3
    },
    {
        88, L"Game Trainer Creation", Tier::Four,
        55, 45, 1, 0,
        {{SKILL_WINDOWS_EXPLOITATION, 15}, {SKILL_REVERSE_ENGINEERING, 12}}, 2,
        {}, 0,
        L"Build simple game trainers. WriteProcessMemory, freeze values, infinite health/ammo hacks.",
        false,
        87, 3, DocId::WinAPIDocs  // prereq: Cheat Table Sales x3
    },

    // Tier 3 Game Hacking
    {
        89, L"Anti-Cheat Bypass Dev", Tier::Three,
        400, 1500, 4, 0,
        {{SKILL_WINDOWS_EXPLOITATION, 5}, {SKILL_REVERSE_ENGINEERING, 4}}, 2,
        {{SKILL_WINDOWS_EXPLOITATION, 12}, {SKILL_REVERSE_ENGINEERING, 10}}, 2,
        L"Bypass EAC, BattlEye, Vanguard. Kernel driver work, manual mapping, syscall hooking.",
        false,
        88, 5, DocId::WinAPIDocs  // prereq: Game Trainer x5
    },
    {
        90, L"Game Exploit Sales", Tier::Three,
        250, 2000, 4, 0,
        {{SKILL_REVERSE_ENGINEERING, 4}, {SKILL_WINDOWS_EXPLOITATION, 3}}, 2,
        {{SKILL_REVERSE_ENGINEERING, 12}}, 1,
        L"Sell game exploits, aimbots, wallhacks on underground forums. Subscription model.",
        false,
        89, 3, DocId::COUNT  // prereq: Anti-Cheat Bypass x3
    },

    // Tier 2 Game Hacking
    {
        91, L"Kernel Anti-Cheat Rootkit", Tier::Two,
        1200, 20000, 10, 0,
        {{SKILL_WINDOWS_EXPLOITATION, 7}, {SKILL_REVERSE_ENGINEERING, 6}}, 2,
        {{SKILL_WINDOWS_EXPLOITATION, 30}, {SKILL_REVERSE_ENGINEERING, 25}}, 2,
        L"Develop kernel-level cheats that survive anti-cheat updates. DKOM, hypervisor hooking.",
        false,
        89, 5, DocId::ReverseEngineeringManuals
    },
    {
        92, L"Game Server Exploitation", Tier::Two,
        900, 15000, 8, 0,
        {{SKILL_WINDOWS_EXPLOITATION, 5}, {SKILL_NETWORK_ENGINEERING, 4}, {SKILL_REVERSE_ENGINEERING, 4}}, 3,
        {{SKILL_WINDOWS_EXPLOITATION, 25}, {SKILL_NETWORK_ENGINEERING, 18}}, 2,
        L"Exploit game server vulnerabilities. Item duplication, rank manipulation, economy crashes.",
        false,
        -1, 0, DocId::RFCNetworkDocs
    },

    // Tier 1 Game Hacking
    {
        93, L"Anti-Cheat Framework Dev", Tier::One,
        4800, 200000, 20, 0,
        {{SKILL_WINDOWS_EXPLOITATION, 8}, {SKILL_REVERSE_ENGINEERING, 8}}, 2,
        {{SKILL_WINDOWS_EXPLOITATION, 45}, {SKILL_REVERSE_ENGINEERING, 42}}, 2,
        L"Build commercial cheat frameworks sold to thousands. Hypervisor-level evasion, custom kernel drivers.",
        false,
        91, 5, DocId::ReverseEngineeringManuals
    },
    {
        94, L"Game Engine Zero-Day", Tier::One,
        7200, 500000, 30, 0,
        {{SKILL_REVERSE_ENGINEERING, 8}, {SKILL_WINDOWS_EXPLOITATION, 7}}, 2,
        {{SKILL_REVERSE_ENGINEERING, 55}, {SKILL_WINDOWS_EXPLOITATION, 48}}, 2,
        L"Discover zero-days in Unreal Engine, Unity, Source. Full RCE through game clients.",
        false,
        93, 3, DocId::ReverseEngineeringManuals
    },

    // ========================================================================
    // CONTRACTOR / MERCENARY JOBS (Jobs 95-102)
    // Unlocked via Cyber Veteran perk (military route completion)
    // FAILURE = JAIL. Lose all money and reputation. Keep skills.
    // ========================================================================
    {
        95, L"Foreign Intelligence Agency Standup", Tier::One,
        7200, 2000000, 100, 25,
        {{SKILL_NETWORK_ENGINEERING, 8}, {SKILL_OPSEC, 8}, {SKILL_SOCIAL_ENGINEERING, 5}}, 3,
        {{SKILL_NETWORK_ENGINEERING, 50}, {SKILL_OPSEC, 45}}, 2,
        L"Stand up a signals intelligence apparatus for a foreign government. Train their operators, deploy collection infrastructure. Just like Project Raven.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        96, L"Intelligence Agency Hacker-for-Hire", Tier::One,
        3600, 800000, 50, 20,
        {{SKILL_WEB_APP_SECURITY, 6}, {SKILL_NETWORK_ENGINEERING, 6}, {SKILL_OPSEC, 5}}, 3,
        {{SKILL_WEB_APP_SECURITY, 40}, {SKILL_NETWORK_ENGINEERING, 35}}, 2,
        L"Contract offensive cyber operator for a three-letter agency. Targets assigned, no questions asked.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        97, L"Intelligence Agency Malware Dev", Tier::One,
        4800, 1200000, 75, 22,
        {{SKILL_WINDOWS_EXPLOITATION, 8}, {SKILL_REVERSE_ENGINEERING, 7}, {SKILL_OPSEC, 5}}, 3,
        {{SKILL_WINDOWS_EXPLOITATION, 50}, {SKILL_REVERSE_ENGINEERING, 45}}, 2,
        L"Develop nation-state grade implants and C2 frameworks. Stuxnet-class tools for government buyers.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        98, L"Contracted Royal Bank Hack", Tier::One,
        5400, 1500000, 80, 28,
        {{SKILL_FINANCIAL_SYSTEMS, 7}, {SKILL_DATABASE_SYSTEMS, 6}, {SKILL_OPSEC, 6}}, 3,
        {{SKILL_FINANCIAL_SYSTEMS, 45}, {SKILL_DATABASE_SYSTEMS, 40}}, 2,
        L"Hack into international banks to trace fund flows for foreign royalty. Track dissidents, freeze assets, locate targets.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        99, L"Corporate Cyber Access Bridge", Tier::One,
        3000, 600000, 40, 18,
        {{SKILL_NETWORK_ENGINEERING, 6}, {SKILL_WINDOWS_EXPLOITATION, 5}, {SKILL_OPSEC, 5}}, 3,
        {{SKILL_NETWORK_ENGINEERING, 40}, {SKILL_OPSEC, 35}}, 2,
        L"Establish persistent covert access into a Fortune 500 corporation for a foreign intelligence service.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        100, L"Foreign Military Network Breach", Tier::One,
        6000, 1800000, 90, 30,
        {{SKILL_NETWORK_ENGINEERING, 8}, {SKILL_EMBEDDED_RTOS, 6}, {SKILL_CRYPTOGRAPHY, 6}}, 3,
        {{SKILL_NETWORK_ENGINEERING, 50}, {SKILL_EMBEDDED_RTOS, 40}}, 2,
        L"Penetrate a foreign military's classified network. Exfiltrate operational plans, troop movements, weapons specs.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        101, L"Regime Surveillance Platform Dev", Tier::One,
        4200, 1000000, 60, 24,
        {{SKILL_MOBILE_PLATFORMS, 7}, {SKILL_TELECOMMUNICATIONS, 6}, {SKILL_OPSEC, 5}}, 3,
        {{SKILL_MOBILE_PLATFORMS, 45}, {SKILL_TELECOMMUNICATIONS, 40}}, 2,
        L"Build mass surveillance infrastructure for an authoritarian regime. Monitor, intercept, and track an entire population.",
        false,
        -1, 0, DocId::COUNT
    },
    {
        102, L"Diplomatic Communications Intercept", Tier::One,
        5400, 1600000, 85, 26,
        {{SKILL_CRYPTOGRAPHY, 7}, {SKILL_TELECOMMUNICATIONS, 7}, {SKILL_OPSEC, 6}}, 3,
        {{SKILL_CRYPTOGRAPHY, 48}, {SKILL_TELECOMMUNICATIONS, 42}}, 2,
        L"Intercept and decrypt diplomatic communications between embassies. Crack encrypted channels, plant implants in secure phones.",
        false,
        -1, 0, DocId::COUNT
    },
};

static const int g_NumJobs = sizeof(g_JobCatalog) / sizeof(g_JobCatalog[0]);

int GetJobCount()
{
    return g_NumJobs;
}

const JobDef& GetJobDef(int index)
{
    if (index < 0 || index >= g_NumJobs)
    {
        return g_JobCatalog[0];
    }
    return g_JobCatalog[index];
}

void InitJobCatalog()
{
    // Jobs are statically initialized, nothing to do
}

// ============================================================================
// Documentation Catalog
// ============================================================================

static DocStudyDef g_DocCatalog[] = {
    { DocId::WinAPIDocs, L"WinAPI Documentation", 45, 500, {SkillTraining{SKILL_WINDOWS_EXPLOITATION, 10}, {}}, 1, L"Study Windows API internals: processes, memory, PE format." },
    { DocId::RFCNetworkDocs, L"RFC & Networking Docs", 35, 0, {SkillTraining{SKILL_NETWORK_ENGINEERING, 10}, {}}, 1, L"Read RFCs on TCP/IP, DNS, BGP, routing protocols." },
    { DocId::UNIXManPages, L"UNIX Man Pages", 35, 0, {SkillTraining{SKILL_UNIX_LINUX, 10}, {}}, 1, L"Study UNIX system calls, shell internals, file permissions." },
    { DocId::WebSecurityRefs, L"Web Security References", 25, 0, {SkillTraining{SKILL_WEB_APP_SECURITY, 10}, {}}, 1, L"OWASP guides, SQL injection references, XSS cheat sheets." },
    { DocId::ReverseEngineeringManuals, L"Reverse Engineering Manuals", 60, 1000, {SkillTraining{SKILL_REVERSE_ENGINEERING, 10}, {}}, 1, L"Study disassembly, debugging, binary analysis." },
    { DocId::CryptoProtocolDocs, L"Crypto Protocol Docs", 45, 500, {SkillTraining{SKILL_CRYPTOGRAPHY, 10}, {}}, 1, L"Study encryption schemes, blockchain internals, key exchange." },
    { DocId::SocialEngineeringPlaybook, L"Social Engineering Playbook", 35, 200, {SkillTraining{SKILL_SOCIAL_ENGINEERING, 10}, {}}, 1, L"Pretexting frameworks, influence psychology, OSINT techniques." },
    { DocId::EmbeddedSystemsReference, L"Embedded Systems Reference", 45, 1500, {SkillTraining{SKILL_EMBEDDED_RTOS, 10}, {}}, 1, L"RTOS internals, JTAG/SWD debugging, firmware extraction." },
    { DocId::EvilNginxDocs, L"EvilNginx & Reverse Proxy Phishing", 35, 300, {SkillTraining{SKILL_SOCIAL_ENGINEERING, 5}, SkillTraining{SKILL_WEB_APP_SECURITY, 5}}, 2, L"Reverse proxy phishing with EvilNginx2. Transparent credential harvesting, session hijacking." },
};

// Intel market items are defined in src/data/strings.cpp

int GetDocCount() { return _countof(g_DocCatalog); }
const DocStudyDef& GetDocDef(int index)
{
    if (index < 0 || index >= static_cast<int>(_countof(g_DocCatalog)))
    {
        return g_DocCatalog[0];
    }
    return g_DocCatalog[index];
}

// ============================================================================
// Boss Hack Definitions
// ============================================================================

static BossHackDef g_BossHacks[NUM_BOSS_HACKS] =
{
    // Boss 0: Operation Marketplace (T4->T3)
    // Target: DarkBazaar underground market
    // Skills: Web App Security, Database Systems, OPSEC, Social Engineering
    {
        L"Operation Marketplace",
        L"DarkBazaar Underground Market",
        Tier::Three,
        // 4 stages with terminal output
        {
            // Stage 1: Reconnaissance
            {
                L"Reconnaissance",
                SKILL_WEB_APP_SECURITY,
                8,
                { L"> dirb https://darkbazaar.onion /usr/share/wordlists/common.txt",
                  L"  FOUND: /admin/ (Status: 302 -> /admin/login)",
                  L"  FOUND: /api/v2/ (Status: 200)",
                  L"  FOUND: /backup/ (Status: 403)",
                  L"> curl -s https://darkbazaar.onion/api/v2/status | jq .",
                  L"  Server: nginx/1.21.3 | PHP 8.1 | MariaDB detected" },
                6
            },
            // Stage 2: Exploitation
            {
                L"Exploitation",
                SKILL_WEB_APP_SECURITY,
                10,
                { L"> sqlmap -u \"darkbazaar.onion/api/v2/search?q=test\" --dbs",
                  L"  [INFO] testing connection to target URL...",
                  L"  [INFO] GET parameter 'q' is vulnerable. Type: UNION query",
                  L"  available databases: darkbazaar_prod, darkbazaar_users",
                  L"> sqlmap --dump -D darkbazaar_users -T admins" },
                5
            },
            // Stage 3: Credential Extraction
            {
                L"Credential Extraction",
                SKILL_DATABASE_SYSTEMS,
                8,
                { L"  [5 entries]",
                  L"  +----+----------+----------------------------------+",
                  L"  | id | username | password_hash                    |",
                  L"  +----+----------+----------------------------------+",
                  L"  |  1 | d4rkl0rd | $2b$12$rK8z... [bcrypt]         |",
                  L"> hashcat -m 3200 -a 0 hashes.txt rockyou.txt" },
                6
            },
            // Stage 4: Cover Tracks
            {
                L"Cover Tracks",
                SKILL_OPSEC,
                6,
                { L"> ssh -o ProxyCommand=\"ncat --proxy socks5h://127.0.0.1:9050\" admin@darkbazaar.onion",
                  L"  Connected. Wiping access logs...",
                  L"> find /var/log -name '*.log' -exec truncate -s 0 {} \\;",
                  L"  Logs cleared. Rotating SSH keys...",
                  L"> exit" },
                5
            },
            {} // 5th stage unused
        },
        4, // numStages
        // Prep options
        {
            { L"Recon Scan", 2000, 120, 5 },
            { L"Buy Exploit", 5000, 60, 8 },
            { L"Bribe Insider", 3000, 180, 7 },
        },
        5000,    // success cash reward
        50,      // success rep reward
        300,     // cooldown (5 min)
        L"INTRUSION DETECTED! DarkBazaar admins traced your connection. They're coming for you.",
        L"Operation Marketplace complete. DarkBazaar admin credentials and user database exfiltrated."
    },

    // Boss 1: Operation Blue Shield (T3->T2)
    // Target: Metropolitan Police Department
    {
        L"Operation Blue Shield",
        L"Metro City Police Department",
        Tier::Two,
        {
            // Stage 1: Network Infiltration
            {
                L"Network Infiltration",
                SKILL_NETWORK_ENGINEERING,
                18,
                { L"> nmap -sV -O -Pn 10.42.7.0/24",
                  L"  Discovered 47 hosts. Scanning services...",
                  L"  10.42.7.12 - Cisco ASA 5520 (ASDM 7.4)",
                  L"  10.42.7.50 - Windows Server 2019 (RDP open)",
                  L"  10.42.7.100 - MSSQL Server 2017",
                  L"> Exploiting CVE-2023-20269 on ASA gateway..." },
                6
            },
            // Stage 2: Privilege Escalation
            {
                L"Privilege Escalation",
                SKILL_WINDOWS_EXPLOITATION,
                15,
                { L"> PsExec.exe \\\\10.42.7.50 -u guest -p \"\" cmd",
                  L"  Escalating via PrintNightmare (CVE-2021-1675)...",
                  L"  [+] Got SYSTEM on MPPD-DC01",
                  L"> mimikatz.exe \"sekurlsa::logonpasswords\" exit",
                  L"  Domain Admin: MPPD\\svc_backup (NTLM: 4a8c...)" },
                5
            },
            // Stage 3: Database Access
            {
                L"Records Deletion",
                SKILL_DATABASE_SYSTEMS,
                16,
                { L"> sqlcmd -S 10.42.7.100 -U sa -P [cracked]",
                  L"  Connected to MPPD_CriminalRecords",
                  L"  SELECT COUNT(*) FROM dbo.Suspects WHERE status='ACTIVE'",
                  L"  (42,891 rows)",
                  L"> DELETE FROM dbo.Suspects WHERE suspect_id IN (...)",
                  L"  (1 row affected) - Your records purged" },
                6
            },
            // Stage 4: Anti-Forensics
            {
                L"Anti-Forensics",
                SKILL_FORENSIC_EVASION,
                14,
                { L"> Clearing Windows Event Logs...",
                  L"  wevtutil cl Security && wevtutil cl System",
                  L"> Timestomping modified files...",
                  L"> Removing Prefetch and SRUM data...",
                  L"  Forensic artifacts eliminated. Disconnecting." },
                5
            },
            {} // 5th stage unused
        },
        4, // numStages
        {
            { L"Recon Scan", 10000, 120, 5 },
            { L"Buy Exploit", 25000, 60, 8 },
            { L"Bribe Insider", 15000, 180, 7 },
        },
        25000, 200, 600,
        L"ALERT! MPPD Cyber Crime Unit traced the intrusion. A federal investigation has been opened.",
        L"Operation Blue Shield complete. Criminal records purged. You're a ghost in the system."
    },

    // Boss 2: Operation Red Vault (T2->T1)
    // Target: CrowdShield cybersecurity firm
    {
        L"Operation Red Vault",
        L"CrowdShield Cybersecurity",
        Tier::One,
        {
            // Stage 1: Initial Access
            {
                L"Perimeter Breach",
                SKILL_REVERSE_ENGINEERING,
                28,
                { L"> Analyzing CrowdShield VPN client binary...",
                  L"  Ghidra: Found hardcoded API key in .rdata section",
                  L"  Decompiling auth module... CVE candidate found",
                  L"  Buffer overflow in certificate validation",
                  L"> Crafting exploit payload for VPN gateway..." },
                5
            },
            // Stage 2: EDR Bypass
            {
                L"EDR Evasion",
                SKILL_WINDOWS_EXPLOITATION,
                30,
                { L"> Their own EDR product is watching everything...",
                  L"  Loading custom syscall stub loader...",
                  L"  Direct NtCreateThreadEx via Hell's Gate",
                  L"  [+] Shellcode executing in ntdll.dll context",
                  L"  [+] EDR hooks bypassed. Undetected." },
                5
            },
            // Stage 3: Tooling Exfil
            {
                L"Arsenal Theft",
                SKILL_OPSEC,
                25,
                { L"> Enumerating \\\\internal-git\\offensive-tools\\",
                  L"  /0days/         - 12 unreleased exploits",
                  L"  /implants/      - Custom C2 framework source",
                  L"  /red-team-ops/  - Client engagement tools",
                  L"> tar czf /tmp/.cache.gz offensive-tools/",
                  L"> Exfiltrating via DNS tunneling (53/udp)..." },
                6
            },
            // Stage 4: Attribution Evasion
            {
                L"Attribution Evasion",
                SKILL_OPSEC,
                27,
                { L"> Planting false flag indicators...",
                  L"  Injecting Mandarin comments in dropped tools",
                  L"  Spoofing C2 to APT41-associated infrastructure",
                  L"> Wiping all forensic artifacts...",
                  L"  Operation complete. Let them chase phantoms." },
                5
            },
            {} // 5th stage unused
        },
        4, // numStages
        {
            { L"Recon Scan", 50000, 120, 5 },
            { L"Buy Exploit", 100000, 60, 8 },
            { L"Bribe Insider", 75000, 180, 7 },
        },
        100000, 500, 900,
        L"CrowdShield detected the breach and published an incident report naming your TTPs. You're burned.",
        L"Operation Red Vault complete. CrowdShield's entire offensive arsenal is now yours."
    },

    // Boss 3: Operation Ghost Protocol (T1->Shadow Broker)
    // Target: National Security Agency
    {
        L"Operation Ghost Protocol",
        L"National Security Agency (NSA)",
        Tier::Four,
        {
            // Stage 1: Air-Gap Breach
            {
                L"Air-Gap Penetration",
                SKILL_EMBEDDED_RTOS,
                40,
                { L"> Deploying modified firmware to target USB devices...",
                  L"  BadUSB payload: HID injection + covert storage",
                  L"  Waiting for device insertion into classified network...",
                  L"  [SIGNAL] Device connected on NSANet segment 7",
                  L"  [+] Beacon established via RF side-channel" },
                5
            },
            // Stage 2: Internal Network
            {
                L"NSANet Traversal",
                SKILL_NETWORK_ENGINEERING,
                42,
                { L"> Internal scan via implant relay...",
                  L"  Pivoting through JWICS gateway (TS/SCI)",
                  L"  Locating XKEYSCORE terminal servers...",
                  L"  Found: Analyst workstations with SCI access",
                  L"  Deploying memory-resident implant..." },
                5
            },
            // Stage 3: Data Exfiltration
            {
                L"Intelligence Exfiltration",
                SKILL_CRYPTOGRAPHY,
                38,
                { L"> Accessing PRISM data stores...",
                  L"  Downloading NOC list (field agent identities)",
                  L"  Encrypting payload: AES-256-GCM + steganography",
                  L"  Embedding in routine network traffic patterns",
                  L"  [+] 2.3GB exfiltrated over 72 simulated hours" },
                5
            },
            // Stage 4: Ghost Exit
            {
                L"Ghost Exit",
                SKILL_OPSEC,
                45,
                { L"> Purging all implant artifacts...",
                  L"  Overwriting MBR timestamps on touched systems",
                  L"  Restoring original firmware checksums",
                  L"  Routing final C2 through 14 sovereign jurisdictions",
                  L"  Connection terminated. No evidence remains." },
                5
            },
            // Stage 5: Verification
            {
                L"Dead Drop Verification",
                SKILL_SOCIAL_ENGINEERING,
                35,
                { L"> Verifying dead drop receipt...",
                  L"  Signal confirmed from broker network",
                  L"  Payment: 50 BTC to tumbled wallet",
                  L"  NOC list authenticated by three agencies",
                  L"  You are now the Shadow Broker." },
                5
            }
        },
        5, // 5 stages for the final boss
        {
            { L"Recon Scan", 200000, 120, 5 },
            { L"Buy Exploit", 500000, 60, 8 },
            { L"Bribe Insider", 300000, 180, 7 },
        },
        1000000, 2000, 1200,
        L"NSA TAO division traced the intrusion. Every intelligence agency on the planet is now hunting you.",
        L"Operation Ghost Protocol complete. You have ascended. You are the Shadow Broker."
    },
};

const BossHackDef& GetBossHackDef(int index)
{
    if (index < 0 || index >= NUM_BOSS_HACKS)
    {
        return g_BossHacks[0];
    }
    return g_BossHacks[index];
}
