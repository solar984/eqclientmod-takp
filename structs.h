#include "common.h"
#pragma pack(push, 1)

struct CXRect
{
	int x1;
	int x2;
	int y1;
	int y2;
};

struct EQPlayer;
struct EQ_PC;
struct EQ_Equipment;

struct EQ_Spell
{
	int Id;
	float Range;
	float AoeRange;
	float PushBack;
	float PushUp;
	int CastTime;
	int RecoveryTime;
	int RecastTime;
	int DurationFormula;
	int Duration;
	int AoeDuration;
	__int16 Mana;
	__int16 Base[12];
	__int16 Max[12];
	__int16 BookIcon;
	__int16 GemIcon;
	__int16 ReagentId[4];
	__int16 ReagentCount[4];
	__int16 NoexpendReagent[4];
	char Calc[12];
	char LightType;
	char BuffType;
	char Activated;
	char ResistType;
	char Attribute[12];
	char TargetType;
	char FizzleAdjustment;
	char SkillType;
	char ZoneType;
	char EnvironmentType;
	char TimeOfDay;
	char Level[32];
	char CastingAnim;
	char TargetAnim;
	char TravelType;
	char disallow_sit;
	char uninterruptable;
	char dot_stacking_exempt;
	char deleteable;
	char unknown_char1;
	int recourse_spell_id;
	int no_partial_resist;
	char field140;
	char field139;
	char align_pad[2];
	char *Name;
	char *Player1;
	char *TeleportZone;
	char *YouCast;
	char *OtherCasts;
	char *CastOnYou;
	char *CastOnOther;
	char *SpellFades;
	int SPAPtr;
	int SpellAffectIndex;
	int SpellEffectPtr;
	int spellanim;
	int deities;
	char npc_no_cast;
	char align_pad2[1];
	short ai_pt_bonus;
	int new_icon;
	int ResistDiff;
};


struct SpellBuff_Struct
{
	/*000*/uint8  bufftype;		// Comment: 0 = Buff not visible, 1 = Visible and permanent buff(Confirmed ) , 2 = Visible and timer on(Confirmed ) 
	/*001*/uint8  level;			// Comment: Level of person who casted buff
	/*002*/uint8  bard_modifier;	// Comment: this seems to be the bard modifier, it is normally 0x0A because we set in in the CastOn_Struct when its not a bard, else its the instrument mod
	/*003*/uint8  activated;	// Copied from spell data to buff struct.  Only a few spells have this set to 1, the rest are 0
	/*004*/uint16 spellid;		// spell id
	/*006*/uint16 duration;		// Duration in ticks
	/*008*/uint16 counters;		// rune amount, poison/disease/curse counters
	/*010*/
};

struct Color_Struct
{
	union
	{
		struct
		{
			uint8	blue;
			uint8	green;
			uint8	red;
			uint8	use_tint;	// if there's a tint this is FF
		} rgb;
		uint32 color;
	};
};


struct EQPMInfo
{
	int32 actor_ptr;
	int32 light_ptr;
	char modelname[64];
	float floorheight;
	float ceilingheight;
	int32 emitter_ptr;
	float unk84; // 0xEC4ECB8F
	int32 time88;
#ifdef MAC
	int32 time92;
#endif
	int32 timerOneSecTick;
	int32 timerSixSecTick;
	int32 time104;
	int32 unk108;
	int32 unk112;
	int32 unk116;
	int32 unk120;
	int32 timerEighteenSecTick;
	int32 time128; // this is only on mac client i think? seems misaligned after this member without this exclusion
	int16 LevitationMovementCounter;
	int16 DrunkMovementCounter;
	int16 unk136;
	int16 unk138;
	float DrunkMovementModifier;
	float LevitationMovementModifier;
	int8 IsAffectedByGravity;
	int8 unk149;
	int8 unk150;
	int8 unk151;
	int32 unk152;
	int32 unk156;
	int32 unk160;
	int8 IsSwimmingUnderwater;
	int8 SwimmingWaterType;
	int8 SwimmingFeetTouchingWater;
	int8 SwimmingUnknown1;
	int8 SwimmingUnknown2;
	int8 SwimmingUnknown3;
	int8 SwimmingUnknown4;
	int8 SwimmingUnknown5;
	float EverQuest14_MovementFriction1; // 43
	float EverQuest15_MovementFriction2;
	float JumpHeightModifier;
	float flt184;
	float flt188;
	float movementSpeed1;
	float unk196;
	int32 stunnedUntilTime;
	int32 time204;
	int32 casting_spell_time;
	int32 unk212;
	int32 unk216;
	int32 spellRecoveryTime[10];
	int32 pad1[24];
	int32 spellGlobalRecoveryTime;
	uint32 unk360; // 90
	uint32 unk364; // 91
	uint32 unk368; // 92
	uint32 unk372; // 93
	uint32 unk376; // 94
	//int32 unk380; // 95
	uint32 intimidateAnimationTimer;
	char pad388[16];
	char intimidateAnimationSequence;
	char blackScreen;
	char pad405[2];
	EQPlayer *vehicle_boat;
	EQPlayer *player1_MyMount;
	EQPlayer *player2_MyRider;
	//char pad396[224];
	//char pad396[38];
	char pad396[20];
	float sneakmod;
	int invisible;
	char pad424[10];
	int16 petEntityId;
	char pad436[184];
	int16 casting_spell_id;
	int8 casting_slot;
	int8 unk649;
	int16 bard_singing_mod;
	int16 unk652;
	//char pad654[182];

	/* 0x0284 */ struct _Model *Model;
	/* 0x0288 */ struct _ModelBone *ModelBoneHeadPoint;
	/* 0x028C */ struct _ModelBone *ModelBoneHead;
	/* 0x0290 */ struct _ModelBone *ModelBoneUnknown;
	/* 0x0294 */ struct _ModelBone *ModelBoneRightPoint;
	/* 0x0298 */ struct _ModelBone *ModelBoneLeftPoint;
	/* 0x029C */ struct _ModelBone *ModelBoneShieldPoint;
	/* 0x02A0 */ uint8_t Unknown02A0[68];

	//char pad654[96];

	EQPlayer *followedPlayer;
	//char pad754[82];
	float headingToDest;
	float headingToDestInverse;
	float distanceToDest;
	//char pad766[70];
	char pad766[40];
	int32 active_discipline;
	char pad810[24];
	char aa_values_by_id[227];
	char pad1059[9];
	int32 casting_time;
	int32 casting_item_invslotnum;
	int32 berserk;
	char pad1080[8];
	/* 0x0438 */ uint32_t IsLookingForGroup; // LFG
	/* 0x043C */ uint32_t IsTrader;
	char pad1087[73];
};

struct AA_Array
{
	uint8 AA;
	uint8 value;
};
#define pp_inventory_size 30
#define pp_containerinv_size 80
#define pp_cursorbaginventory_size 10
#define pp_bank_inv_size 8

// this one looks pretty good, it's sent to client during OP_ZoneEntry
struct EQPlayer
{
	/*0000*/	//uint8	checksum[4];		// Checksum
	/*0004*/	uint8	type;		// ***Placeholder
	/*0005*/	char	name[64];			// Name
	/*0069*/	uint8	sze_unknown0069;	// ***Placeholder
	/*0070*/	uint16	unknown0070;		// ***Placeholder
	/*0072*/	uint32	zoneID;				// Current Zone
	/*0076*/	float	y_pos;				// Y Position
	/*0080*/	float	x_pos;				// X Position
	/*0084*/	float	z_pos;				// Z Position
	/*0088*/	float	heading;
	/*0092*/	float	physicsinfo[8];
	/*0124*/	EQPlayer *prev;
	/*0128*/	EQPlayer *next;
	/*0132*/	int32	corpse;
	/*0136*/	EQPMInfo *LocalInfo;
	/*0140*/	EQ_PC *My_Char;
	/*0144*/	float	view_height;
	/*0148*/	float	sprite_oheight;
	/*0152*/	uint16	entityId;
	/*0154*/	uint16	petOwnerId;
	/*0156*/	uint32	max_hp;
	/*0160*/	uint32	curHP;
	/*0164*/	uint16	GuildID;			// Guild ID Number
	/*0166*/	uint8	socket[6];		// ***Placeholder
	/*0172*/	uint8	NPC;
	/*0173*/	uint8	class_;				// Class
	/*0174*/	uint16	race;				// Race
	/*0176*/	uint8	gender;				// Gender
	/*0177*/	uint8	level;				// Level
	/*0178*/	uint8	invis;
	/*0179*/	uint8	sneaking;
	/*0180*/	uint8	pvp;				// PVP Flag
	/*0181*/	uint8	anim_type;
	/*0182*/	uint8	light;
	/*0183*/	int8	face;				// Face Type
	/*0184*/    uint16  equipment[9]; // Array elements correspond to struct equipment above
	/*0202*/	uint16	unknown; //Probably part of equipment
	/*0204*/	Color_Struct equipcolors[9]; // Array elements correspond to struct equipment_colors above
	/*0240*/	uint32	bodytexture;	// Texture (0xFF=Player - See list of textures for more)
	/*0244*/	float	size;
	/*0248*/	float	width;
	/*0252*/	float	length;
	/*0256*/	uint32	helm;
	/*0260*/	float	walkspeed;			// Speed when you walk
	/*0264*/	float	runspeed;			// Speed when you run
	/*0268*/	int8	LD;
	/*0269*/	int8	GM;
	/*0270*/	int16	flymode;
	/*0272*/	int32	bodytype;
	/*0276*/	int32	view_player;
	/*0280*/	uint8	anon;				// Anon. Flag
	/*0281*/	uint16	avatar;
	/*0283*/	uint8	AFK;
	/*0284*/	uint8	summoned_pc;
	/*0285*/	uint8	title;
	/*0286*/	uint8	extra[18];	// ***Placeholder (At least one flag in here disables a zone point or all)
	/*0304*/	char	Surname[32];		// Lastname (This has to be wrong.. but 70 is to big =/..)
	/*0336*/	uint16  guildrank;
	/*0338*/	uint16	deity;				// Diety (Who you worship for those less literate)
	/*0340*/	int8	animation;		// ***Placeholder
	/*0341*/	uint8	haircolor;			// Hair Color
	/*0342*/	uint8	beardcolor;			// Beard Color
	/*0343*/	uint8	eyecolor1;			// Left Eye Color
	/*0344*/	uint8	eyecolor2;			// Right Eye Color
	/*0345*/	uint8	hairstyle;			// Hair Style
	/*0346*/	uint8	beard;				// AA Title
	/*0347*/	uint32	SerialNumber;
	/*0351*/	char	m_bTemporaryPet[4];
	/*0355*/	uint8	void_;
	/*0356*/
};
struct ItemProperties_Struct
{

	/*000*/	uint8	unknown01[2];
	/*002*/	int8	charges;				// Comment: signed int because unlimited charges are -1
	/*003*/	uint8	unknown02[7];
	/*010*/
};
struct EQ_Equipment
{
	/*0000*/ char		Name[64];			// Name of item
	/*0064*/ char		Lore[80];			// Lore text
	/*0144*/ char		IDFile[30];			// This is the filename of the item graphic when held/worn.
	/*0174*/ uint8		Weight;				// Weight of item
	/*0175*/ uint8		NoRent;				// Nosave flag 1=normal, 0=nosave, -1=spell?
	/*0176*/ uint8		NoDrop;				// Nodrop flag 1=normal, 0=nodrop, -1=??
	/*0177*/ uint8		Size;				// Size of item
	/*0178*/ int16		ItemClass;
	/*0180*/ int16		ID;					// Record number. Confirmed to be signed.
	/*0182*/ uint16		Icon;				// Icon Number
	/*0184*/ int16		equipSlot;			// Current slot location of item
	/*0186*/ uint8		unknown0186[2];		// Client dump has equipSlot/location as a short so this is still unknown
	/*0188*/ int32		Slots;				// Slots where this item is allowed
	/*0192*/ int32		Price;				// Item cost in copper
	/*0196*/ float		cur_x;				//Here to 227 are named from client struct dump.
	/*0200*/ float		cur_y;
	/*0204*/ float		cur_z;
	/*0208*/ float		heading;
	/*0212*/ uint32		inv_refnum;			// Unique serial. This is required by apply poison.
	/*0216*/ int16		log;
	/*0218*/ int16		loot_log;
	/*0220*/ int16		avatar_level;		//Usually 01, sometimes seen as FFFF, once as 0.
	/*0222*/ int16		bottom_feed;
	/*0224*/ uint32		poof_item;
	union
	{
		struct
		{
			// 0228- have different meanings depending on flags
			/*0228*/ int8		AStr;				// Strength
			/*0229*/ int8		ASta;				// Stamina
			/*0230*/ int8		ACha;				// Charisma
			/*0231*/ int8		ADex;				// Dexterity
			/*0232*/ int8		AInt;				// Intelligence
			/*0233*/ int8		AAgi;				// Agility
			/*0234*/ int8		AWis;				// Wisdom
			/*0235*/ int8		MR;					// Magic Resistance
			/*0236*/ int8		FR;					// Fire Resistance
			/*0237*/ int8		CR;					// Cold Resistance
			/*0238*/ int8		DR;					// Disease Resistance
			/*0239*/ int8		PR;					// Poison Resistance
			/*0240*/ int16		HP;					// Hitpoints
			/*0242*/ int16		Mana;				// Mana
			/*0244*/ int16		AC;					// Armor Class
			/*0246*/ int8		MaxCharges;			// Maximum number of charges, for rechargable? (Sept 25, 2002)
			/*0247*/ uint8      GMFlag;				// GM flag 0  - normal item, -1 - gm item (Sept 25, 2002)
			/*0248*/ uint8		Light;				// Light effect of this item
			/*0249*/ uint8		Delay;				// Weapon Delay
			/*0250*/ uint8		Damage;				// Weapon Damage
			/*0251*/ int8		EffectType1;		// 0=combat, 1=click anywhere w/o class check, 2=latent/worn, 3=click anywhere EXPENDABLE, 4=click worn, 5=click anywhere w/ class check, -1=no effect
			/*0252*/ uint8		Range;				// Range of weapon
			/*0253*/ uint8		ItemType;			// Skill of this weapon, refer to weaponskill chart
			/*0254*/ uint8      Magic;				// Magic flag
			/*0255*/ uint8      EffectLevel1;		// Casting level
			/*0256*/ uint32		Material;			// Material
			/*0260*/ uint32		Color;				// Amounts of RGB in original color
			/*0264*/ int16		Faction;			// Structs dumped from client has this as Faction
			/*0266*/ int16		Effect1;			// SpellID of special effect
			/*0268*/ int32		Classes;			// Classes that can use this item
			/*0272*/ int32		Races;				// Races that can use this item
			/*0276*/ int8		Stackable;			//  1= stackable, 3 = normal, 0 = ? (not stackable)			
		} common;
		struct
		{
			/*0228*/ int16		BookType;			// Type of book (scroll, note, etc)
			/*0230*/ int8		Book;				// Are we a book
			/*0231*/ char		Filename[30];		// Filename of book text on server
			/*0261*/ int32		buffer1[4];			// Not used, fills out space in the packet so ShowEQ doesn't complain.
		} book;
		struct
		{
			/*0228*/ int32		buffer2[10];		// Not used, fills out space in the packet so ShowEQ doesn't complain.
			/*0268*/ uint8		BagType;			//Bag type (obviously)
			/*0269*/ uint8		BagSlots;			// number of slots in container
			/*0270*/ uint8		IsBagOpen;			// 1 if bag is open, 0 if not.
			/*0271*/ uint8		BagSize;			// Maximum size item container can hold
			/*0272*/ uint8		BagWR;				// % weight reduction of container
			/*0273*/ uint32		buffer3;			// Not used, fills out space in the packet so ShowEQ doesn't complain.
		} container;
	};
	/*0277*/ uint8		EffectLevel2;				// Casting level
	/*0278*/ int8		Charges;					// Number of charges (-1 = unlimited)
	/*0279*/ int8		EffectType2;				// 0=combat, 1=click anywhere w/o class check, 2=latent/worn, 3=click anywhere EXPENDABLE, 4=click worn, 5=click anywhere w/ class check, -1=no effect
	/*0280*/ uint16		Effect2;					// spellId of special effect
	/*0282*/ int16		Effect2Duration;			// seen in client decompile being set to duration of Effect2 spell but purpose unknown
	/*0284*/ int16		HouseLockID;				// MSG_REQ_HOUSELOCK
	/*0286*/ uint8		unknown0286[2];
	/*0288*/ float		SellRate;
	/*0292*/ int32		CastTime;					// Cast time of clicky item in miliseconds
	/*0296*/ uint8		unknown0296[12];			// ***Placeholder
	/*0308*/ int32		RecastTime;					// Recast time of clicky item in milliseconds
	/*0312*/ uint16		SkillModType;
	/*0314*/ int16		SkillModValue;
	/*0316*/ int16		BaneDmgRace;
	/*0318*/ int16		BaneDmgBody;
	/*0320*/ uint8		BaneDmgAmt;
	/*0321*/ uint8		unknown0321;
	/*0322*/ uint16		title;
	/*0324*/ uint8		RecLevel;					// max should be 65
	/*0325*/ uint8		RecSkill;					// Max should be 252
	/*0326*/ int16		ProcRate;
	/*0328*/ uint8		ElemDmgType;
	/*0329*/ uint8		ElemDmgAmt;
	/*0330*/ int16		FactionMod1;
	/*0332*/ int16		FactionMod2;
	/*0334*/ int16		FactionMod3;
	/*0336*/ int16		FactionMod4;
	/*0338*/ int16		FactionAmt1;
	/*0340*/ int16		FactionAmt2;
	/*0342*/ int16		FactionAmt3;
	/*0344*/ int16		FactionAmt4;
	/*0346*/ uint16		Void346;
	/*0348*/ int32		Deity;						// Bitmask of Deities that can equip this item
	/*0352*/ int16		ReqLevel;					// Required level
	/*0354*/ int16		BardType;
	/*0356*/ int16		BardValue;
	/*0358*/ int16		FocusEffect;				//Confirmed
	/*0360*/
};
struct EQ_Character
{
public:
	uint8 unknown0004[2];
	char name[64];
	char last_name[66];
	uint32 uniqueGuildID;
	uint8 gender;
	char genderchar[1];
	uint16 race;
	uint16 class_;
	uint16 bodytype;
	uint8 level;
	char levelchar[3];
	uint32 exp;
	int16 points;
	int16 mana;
	int16 cur_hp;
	uint16 status;
	int16 STR;
	int16 STA;
	int16 CHA;
	int16 DEX;
	int16 INT;
	int16 AGI;
	int16 WIS;
	uint8 face;
	uint8 EquipType[9];
	uint32 EquipColor[9];
	int16 inventory[30];
	uint8 languages[32];
	struct ItemProperties_Struct invItemProperties[30];
	struct SpellBuff_Struct buffs[15];
	int16 containerinv[80];
	int16 cursorbaginventory[10];
	struct ItemProperties_Struct bagItemProperties[80];
	struct ItemProperties_Struct cursorItemProperties[10];
	int16 spell_book[256];
	uint8 unknown2374[512];
	int16 mem_spells[8];
	uint8 unknown2886[16];
	uint16 available_slots;
	float y;
	float x;
	float z;
	float heading;
	uint32 position;
	int32 platinum;
	int32 gold;
	int32 silver;
	int32 copper;
	int32 platinum_bank;
	int32 gold_bank;
	int32 silver_bank;
	int32 copper_bank;
	int32 platinum_cursor;
	int32 gold_cursor;
	int32 silver_cursor;
	int32 copper_cursor;
	int32 currency[4];
	int16 skills[100];
	int16 innate_skills[25];
	uint8 air_supply;
	uint8 texture;
	float height;
	float width;
	float length;
	float view_height;
	char boat[32];
	uint8 unknown[60];
	uint8 autosplit;
	uint8 unknown3449[43];
	uint8 expansions;
	uint8 unknown3393[23];
	int32 hunger_level;
	int32 thirst_level;
	struct SpellBuff_Struct npc_buffs[2];
	uint32 zone_id;
	EQPlayer *EQ_Player;
	struct EQ_Equipment *held_item[30];
	uint8 unknown3456[2];
	SpellBuff_Struct npc_extra_buffs[15];
	uint16 buff_caster_entityid[30];
};
struct EQ_PC
{
	/* ***************** */
	/*0000*/	//uint32  checksum;		    // Checksum
	struct EQ_Character EQ_Character;
	/// This is the end of the EQ_Character portion

	/*3784*/	uint32	bind_point_zone[5];
	/*3804*/	float	bind_y[5];
	/*3824*/	float	bind_x[5];
	/*3844*/	float	bind_z[5];
	/*3864*/	float	bind_heading[5];
	/*3884*/	ItemProperties_Struct	bankinvitemproperties[pp_bank_inv_size];
	/*3964*/	ItemProperties_Struct	bankbagitemproperties[pp_containerinv_size];
	/*4764*/	uint32	login_time;
	/*4768*/	int16	bank_inv[pp_bank_inv_size];		// Player Bank Inventory Item Numbers
	/*4784*/	int16	bank_cont_inv[pp_containerinv_size];	// Player Bank Inventory Item Numbers (Bags)
	/*4944*/	uint16	deity;		// ***Placeholder
	/*4946*/	uint16	guild_id;			// Player Guild ID Number
	/*4948*/	uint32  birthday;
	/*4952*/	uint32  lastlogin;
	/*4956*/	uint32  timePlayedMin;
	/*4960*/	int8    thirst_level;
	/*4961*/    int8    hunger_level;
	/*4962*/	int8   fatigue;
	/*4963*/	uint8	pvp;				// Player PVP Flag
	/*4964*/	uint8	level2;
	/*4965*/	uint8	anon;				// Player Anon. Flag
	/*4966*/	uint8	gm;					// Player GM Flag
	/*4967*/	uint8	guildrank;			// Player Guild Rank (0=member, 1=officer, 2=leader)
	/*4968*/    uint8   intoxication;
	/*4969*/	uint8	eqbackground;
	/*4970*/	uint8	unknown4760[2];
	/*4972*/	uint32	spellSlotRefresh[8];
	/*5004*/	uint32	unknown5003;
	/*5008*/	uint32	abilitySlotRefresh;
	/*5012*/	char	groupMembers[6][64];	// Group Members
	/*5396*/	uint8	unknown5396[20];
	/*5416*/	uint32	groupdat;
	/*5420*/	uint32	expAA;				// Post60Exp
	/*5424*/    uint8	title;
	/*5425*/	uint8	perAA;			    // Player AA Percent
	/*5426*/	uint8	haircolor;			// Player Hair Color
	/*5427*/	uint8	beardcolor;			// Player Beard Color
	/*5428*/	uint8	eyecolor1;			// Player Left Eye Color
	/*5429*/	uint8	eyecolor2;			// Player Right Eye Color
	/*5430*/	uint8	hairstyle;			// Player Hair Style
	/*5431*/	uint8	beard;				// Player Beard Type
	/*5432*/	uint8	luclinface;				// Player Face Type mostly 0 in packet
	/*5433*/	uint32	item_material[9];
	/*5469*/	uint8	unknown5469[143]; //item_tint is in here somewhere.
	/*5612*/	AA_Array aa_array[120];
	/*5852*/	uint32	ATR_DIVINE_RES_timer;
	/*5856*/    uint32  ATR_FREE_HOT_timer;
	/*5860*/	uint32	ATR_TARGET_DA_timer;
	/*5864*/	uint32	SptWoodTimer;
	/*5868*/	uint32	DireCharmTimer;
	/*5872*/	uint32	ATR_STRONG_ROOT_timer;
	/*5876*/	uint32	ATR_MASOCHISM_timer;
	/*5880*/	uint32	ATR_MANA_BURN_timer;
	/*5884*/	uint32	ATR_GATHER_MANA_timer;
	/*5888*/	uint32	ATR_PET_LOH_timer;
	/*5892*/	uint32	ExodusTimer;
	/*5896*/	uint32	ATR_MASS_FEAR_timer;
	/*5900*/    uint16  air_remaining;
	/*5902*/    uint16  aapoints;
	/*5904*/	uint32	MGBTimer;
	/*5908*/	uint8   unknown5908[91];
	/*5999*/	int8	mBitFlags[6];
	/*6005*/	uint8	Unknown6004[707];
	/*6712*/	uint32	PoPSpellTimer;
	/*6716*/	uint32	LastShield;
	/*6720*/	uint32	LastModulated;
	/*6724*/	uint8	Unknown6724[1736]; // According to the client, this is all unused/unknown space.
	/*8460*/
};

struct Death_Struct
{
	/*000*/	uint16	spawn_id;
	/*002*/	uint16	killer_id;
	/*004*/	uint16	corpseid;
	/*006*/	uint8	spawn_level;
	/*007*/	uint8   unknown007;
	/*008*/	int16	spell_id;
	/*010*/	uint8	attack_skill;
	/*011*/	uint8   unknonw011;
	/*012*/	int32	damage;
	/*016*/	uint8   is_PC;
	/*017*/	uint8   unknown015[3];
	/*020*/
};

struct SpellManager
{
	EQ_Spell *spells1[4000];
	uint32 spells_crc[4000];
	EQ_Spell *unknown_spell;
};

struct Action_Struct
{
	/*00*/	uint16	target;				// Target entity ID
	/*02*/	uint16	source;				// Caster entity ID
	/*04*/	uint16	level;				// this only does something for spell ids 1252-1266 (potions) and only accepts values 1-60, otherwise the action uses source entity level
	/*06*/	uint16	target_level;		// unused by client
	/*08*/	int32	instrument_mod;		// normally 10, used for bard songs
	/*12*/	float	force;				// push force
	/*16*/	float	sequence;			// push heading
	/*20*/	float	pushup_angle;		// push pitch
	/*24*/	uint8	type;				// 231 for spells
	/*25*/	uint8	unknown25;			// unused by client
	/*26*/	uint16	spell_id_unused;	// extra spell_id, not used by client
	/*28*/	int16	tap_amount;			// used in client for instant, targettype 13 (tap) spells to set the amount that was tapped
	/*30*/	uint16	spell;				// spell_id
	/*32*/	uint8	unknown32;			// 0x00
	/*33*/  uint8	buff_unknown;		// 1 to start then 4 for success
	/*34*/	uint16	unknown34;			// unused by client
};


struct SpawnPositionUpdate_Struct
{
	/*0000*/ uint16	spawn_id;               // Id of spawn to update
	/*0002*/ int8	    anim_type; // ??
	/*0003*/ uint8	heading;                // Heading
	/*0004*/ int8		delta_heading;          // Heading Change
	/*0005*/ int16	y_pos;                  // New X position of spawn
	/*0007*/ int16	x_pos;                  // New Y position of spawn
	/*0009*/ int16	z_pos;                  // New Z position of spawn
	/*0011*/
	struct
	{
		uint32 value;

		// This is our X coordinate, but the client's Y coordinate
		float GetX()
		{
			uint32 vx1 = value >> 22; // 10 bits long, bits 22 through 31
			int32 vx = static_cast<int32>(vx1 & 0x200 ? vx1 | 0xFFFFFC00 : vx1); // extend sign

			// these are multiplied by 16 in client pack_physics()
			return vx * 0.0625f;
		}

		// Our Y coordinate, client's X coordinate
		float GetY()
		{
			uint32 vy1 = value & 0x7FF; // 11 bits long, bits 0 through 10
			int32 vy = static_cast<int32>(vy1 & 0x400 ? vy1 | 0xFFFFF800 : vy1); // extend sign

			return vy * 0.0625f;
		}

		float GetZ()
		{
			uint32 vz1 = (value >> 11) & 0x7FF; // 11 bits long, bits 10 through 21
			int32 vz = static_cast<int32>(vz1 & 0x400 ? vz1 | 0xFFFFF800 : vz1); // extend sign

			return vz * 0.0625f;
		}

		// X and Y are reversed in this function to match the above
		uint32 SetValue(float vx, float vy, float vz)
		{
			value = ((int)(float)(vx * 16.0f) << 22) | (((int)(float)(vz * 16.0f) & 0x7FF) << 11) | (int)(float)(vy * 16.0f) & 0x7FF;

			return value;
		}
	} delta_yzx;
	/*015*/
};


/*
if the cap1 is 0 then it should be initialized to level 255
if the level_required is > 1 then the skill is initialized to level 254 which makes it so you have to train it before it's usable
if the level_required is 1 and the cap1 > 0 then it should be initialized to 0
*/
struct EQ_Skill
{
	/* 0000 */ int32 name_stringid;
	/* 0004 */ int32 reuse_time;
	/* 0008 */ char unk8; // all 0
	/* 0009 */ char can_activate;
	/* 0010 */ char shared_timer; // if it's 2 they share a timer, using one blocks out the others.  not sure about 0 and 1
	/* 0011 */ char level_limited_cap;
	/* 0012 */ char level_required[32]; // one for each class - all blanks except the player classes (1-15)
	/* 0044 */ char difficulty[32];
	/* 0076 */ int16 cap1[32];
	/* 0140 */ int16 base_damage; // backstab, intimidate, bash, monk attacks have values here

	// 104/105 are Baking, Tailoring, Blacksmithing, Fletching, Brewing, Alcohol Tolerance, Jewelry Making, Pottery
	// 116/117 are a bunch of automatic skills that aren't activated
	/* 0142 */ int16 unk142; // icon/string? dragon punch and tail rake override these to get the right UI whatever this is
	/* 0144 */ int16 unk144; // icon/string? dragon punch and tail rake override these to get the right UI whatever this is
	/* 0146 */ int16 pad5; // all 0
	/* 0148 */ float push_force;
	/* 0152 */ int16 cap2[32];
};

/*
** When somebody changes what they're wearing or give a pet a weapon (model changes)
*/
struct WearChange_Struct
{
	/*000*/ uint16 spawn_id;
	/*002*/ uint8  wear_slot_id;
	/*003*/ uint8  unknown03; // This is always used for Primary/Secondary. It also is used on the initial wearchange when a player is entering the zone. It isn't a slot :(
	/*004*/ uint16 material;
	/*006*/ uint16 unknown06;
	/*008*/ uint32 color;
	/*012*/
};

#pragma pack(pop)
