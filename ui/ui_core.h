#ifndef UI_CORE_H
#define UI_CORE_H

//////////////////////////////
// NOTE(hampus): Key

struct UI_Key
{
  U64 u64[1];
  B32
  operator==(UI_Key other)
  {
    B32 result = other.u64[0] == u64[0];
    return result;
  }

  B32
  operator!=(UI_Key other)
  {
    B32 result = !(*this == other);
    return result;
  }
};

//////////////////////////////
// NOTE(hampus): Icons

enum UI_Icon
{
  UI_Icon_Search,
  UI_Icon_Mail,
  UI_Icon_MailAlt,
  UI_Icon_Heart,
  UI_Icon_HeartEmpty,
  UI_Icon_Star,
  UI_Icon_StarEmpty,
  UI_Icon_User,
  UI_Icon_Videocam,
  UI_Icon_Picture,
  UI_Icon_Camera,
  UI_Icon_ThLarge,
  UI_Icon_Th,
  UI_Icon_ThList,
  UI_Icon_Ok,
  UI_Icon_OkCircled,
  UI_Icon_OkCircled2,
  UI_Icon_OkSquared,
  UI_Icon_Cancel,
  UI_Icon_CancelCircled,
  UI_Icon_CancelCircled2,
  UI_Icon_Plus,
  UI_Icon_PlusSquared,
  UI_Icon_PlusSquaredAlt,
  UI_Icon_Minus,
  UI_Icon_MinusCircled,
  UI_Icon_MinusSquared,
  UI_Icon_MinusSquaredAlt,
  UI_Icon_PlusCircled,
  UI_Icon_Help,
  UI_Icon_InfoCircled,
  UI_Icon_Info,
  UI_Icon_Home,
  UI_Icon_Link,
  UI_Icon_Unlink,
  UI_Icon_LinkExt,
  UI_Icon_LinkExtAlt,
  UI_Icon_Attach,
  UI_Icon_Lock,
  UI_Icon_LockOpenAlt,
  UI_Icon_LockOpen,
  UI_Icon_Pin,
  UI_Icon_Eye,
  UI_Icon_EyeOff,
  UI_Icon_Bookmark,
  UI_Icon_BookmarkEmpty,
  UI_Icon_ThumbsUpAlt,
  UI_Icon_ThumbsDownAlt,
  UI_Icon_Download,
  UI_Icon_Upload,
  UI_Icon_Reply,
  UI_Icon_ReplyAll,
  UI_Icon_Forward,
  UI_Icon_Export,
  UI_Icon_ExportAlt,
  UI_Icon_Pencil,
  UI_Icon_PencilSquared,
  UI_Icon_Edit,
  UI_Icon_AttentionAlt,
  UI_Icon_Attention,
  UI_Icon_AttentionCircled,
  UI_Icon_Trash,
  UI_Icon_TrashEmpty,
  UI_Icon_Doc,
  UI_Icon_Docs,
  UI_Icon_DocText,
  UI_Icon_DocInv,
  UI_Icon_DocTextInv,
  UI_Icon_FileImage,
  UI_Icon_Folder,
  UI_Icon_FolderOpen,
  UI_Icon_FolderOpenEmpty,
  UI_Icon_FolderEmpty,
  UI_Icon_Box,
  UI_Icon_Menu,
  UI_Icon_Cog,
  UI_Icon_Sliders,
  UI_Icon_Calendar,
  UI_Icon_CalendarEmpty,
  UI_Icon_Login,
  UI_Icon_Logout,
  UI_Icon_Block,
  UI_Icon_ResizeFull,
  UI_Icon_ResizeFullAlt,
  UI_Icon_ResizeSmall,
  UI_Icon_ResizeVertical,
  UI_Icon_ResizeHorizontal,
  UI_Icon_Move,
  UI_Icon_DownOpen,
  UI_Icon_LeftOpen,
  UI_Icon_RightOpen,
  UI_Icon_UpOpen,
  UI_Icon_AngleLeft,
  UI_Icon_AngleRight,
  UI_Icon_AngleUp,
  UI_Icon_AngleDown,
  UI_Icon_AngleCircledLeft,
  UI_Icon_AngleCircledRight,
  UI_Icon_AngleCircledUp,
  UI_Icon_AngleCircledDown,
  UI_Icon_Down,
  UI_Icon_Left,
  UI_Icon_Right,
  UI_Icon_Up,
  UI_Icon_DownBig,
  UI_Icon_LeftBig,
  UI_Icon_RightBig,
  UI_Icon_UpBig,
  UI_Icon_Cw,
  UI_Icon_Ccw,
  UI_Icon_Desktop,
  UI_Icon_Laptop,
  UI_Icon_Tablet,
  UI_Icon_Mobile,
  UI_Icon_Inbox,
  UI_Icon_AlignLeft,
  UI_Icon_AlignCenter,
  UI_Icon_AlignRight,
  UI_Icon_AlignJustify,
  UI_Icon_List,
  UI_Icon_ListBullet,
  UI_Icon_ListNumbered,
  UI_Icon_Columns,
  UI_Icon_Table,
  UI_Icon_Ellipsis,
  UI_Icon_EllipsisVert,
  UI_Icon_Filter,
  UI_Icon_ChartBar,
  UI_Icon_ChartArea,
  UI_Icon_ChartPie,
  UI_Icon_ChartLine,
  UI_Icon_Circle,
  UI_Icon_CircleEmpty,
  UI_Icon_CircleThin,
  UI_Icon_CheckEmpty,
  UI_Icon_Check,
  UI_Icon_ToggleOn,
  UI_Icon_ToggleOff,
  UI_Icon_Adjust,
  UI_Icon_SortAltUp,
  UI_Icon_SortAltDown,
  UI_Icon_SortNameUp,
  UI_Icon_SortNameDown,
  UI_Icon_SortNumberUp,
  UI_Icon_SortNumberDown,
  UI_Icon_GithubCircled,
  UI_Icon_FacebookOfficial,
  UI_Icon_FacebookSquared,
  UI_Icon_Facebook,
  UI_Icon_LinkedinSquared,
  UI_Icon_Linkedin,
  UI_Icon_Youtube,
  UI_Icon_TwitterSquared,
  UI_Icon_Twitter,

  UI_Icon_COUNT,
};

//////////////////////////////
// NOTE(hampus): Size types

enum UI_SizeKind
{
  UI_SizeKind_Null,
  UI_SizeKind_Pixels,
  UI_SizeKind_TextContent,
  UI_SizeKind_Pct,
  UI_SizeKind_ChildrenSum,
  UI_SizeKind_OtherAxis,
};

struct UI_Size
{
  UI_SizeKind kind;
  F32 value;
  F32 strictness;
};

enum UI_TextAlign
{
  UI_TextAlign_Center,
  UI_TextAlign_Left,
  UI_TextAlign_Right,

  UI_TextAlign_COUNT,
};

//////////////////////////////
// NOTE(hampus): Box types

typedef U32 UI_BoxFlags;
enum
{
  UI_BoxFlag_Disabled = (1 << 0),

  UI_BoxFlag_Clickable = (1 << 1),
  UI_BoxFlag_ViewScroll = (1 << 2),

  UI_BoxFlag_DrawText = (1 << 3),
  UI_BoxFlag_DrawBorder = (1 << 4),
  UI_BoxFlag_DrawBackground = (1 << 5),
  UI_BoxFlag_DrawDropShadow = (1 << 6),
  UI_BoxFlag_HotAnimation = (1 << 7),
  UI_BoxFlag_ActiveAnimation = (1 << 8),
  UI_BoxFlag_FocusAnimation = (1 << 9),

  UI_BoxFlag_AllowOverflowX = (1 << 10),
  UI_BoxFlag_AllowOverflowY = (1 << 11),
  UI_BoxFlag_Clip = (1 << 12),
  UI_BoxFlag_FixedX = (1 << 13),
  UI_BoxFlag_FixedY = (1 << 14),

  UI_BoxFlag_AnimateX = (1 << 15),
  UI_BoxFlag_AnimateY = (1 << 16),
  UI_BoxFlag_AnimateWidth = (1 << 17),
  UI_BoxFlag_AnimateHeight = (1 << 18),
  UI_BoxFlag_AnimateScrollX = (1 << 19),
  UI_BoxFlag_AnimateScrollY = (1 << 20),

  UI_BoxFlag_Focus = (1 << 21),
  UI_BoxFlag_DrawBackgroundOnActiveOrHot = (1 << 22),
  UI_BoxFlag_Commitable = (1 << 23),

  UI_BoxFlag_FlingX = (1 << 24),
  UI_BoxFlag_FlingY = (1 << 25),

  UI_BoxFlag_FixedWidth = (1 << 26),
  UI_BoxFlag_FixedHeight = (1 << 27),

  UI_BoxFlag_FixedPos = UI_BoxFlag_FixedX | UI_BoxFlag_FixedY,
  UI_BoxFlag_FixedDim = UI_BoxFlag_FixedWidth | UI_BoxFlag_FixedHeight,
  UI_BoxFlag_FixedRect = UI_BoxFlag_FixedPos | UI_BoxFlag_FixedDim,

  UI_BoxFlag_AnimatePos = UI_BoxFlag_AnimateX | UI_BoxFlag_AnimateY,
  UI_BoxFlag_AnimateDim = UI_BoxFlag_AnimateWidth | UI_BoxFlag_AnimateHeight,
  UI_BoxFlag_AnimateScroll = UI_BoxFlag_AnimateScrollX | UI_BoxFlag_AnimateScrollY,
};

struct UI_FreeBox
{
  UI_FreeBox *next;
};

#define stack_values                           \
  X(RectColor00, rect_color00, Vec4F32)        \
  X(RectColor10, rect_color10, Vec4F32)        \
  X(RectColor01, rect_color01, Vec4F32)        \
  X(RectColor11, rect_color11, Vec4F32)        \
  X(BorderColor, border_color, Vec4F32)        \
  X(BorderThickness, border_thickness, F32)    \
  X(CornerRadius, corner_radius, Vec4F32)      \
  X(Softness, softness, F32)                   \
  X(HoverCursor, hover_cursor, OS_Cursor)      \
  X(Slice, slice, R_Tex2DSlice)                \
  X(TextColor, text_color, Vec4F32)            \
  X(FixedPos, fixed_pos, Vec2F32)              \
  X(ChildLayoutAxis, child_layout_axis, Axis2) \
  X(BoxFlags, box_flags, UI_BoxFlags)          \
  X(FontTag, font_tag, F_Tag)                  \
  X(FontSize, font_size, U32)                  \
  X(TextPadding, text_padding, Vec2F32)        \
  X(TextAlign, text_align, UI_TextAlign)       \
  X(PrefWidth, pref_width, UI_Size)            \
  X(PrefHeight, pref_height, UI_Size)          \
  X(FixedSize, fixed_size, Vec2F32)

struct UI_Box;
#define UI_CUSTOM_DRAW_FUNCTION(name) void name(UI_Box *root)
typedef UI_CUSTOM_DRAW_FUNCTION(UI_CustomDrawFunction);

struct UI_Box
{
  // hampus: Tree links
  UI_Box *parent;
  UI_Box *first;
  UI_Box *last;
  UI_Box *next;
  UI_Box *prev;

  // hampus: Hash links
  UI_Box *hash_next;
  UI_Box *hash_prev;

  // hampus: Key + generation
  UI_Key key;
  U64 first_build_touched_idx;
  U64 last_build_touched_idx;

  Vec2F32 fixed_pos_animated;

  Vec2F32 fixed_size_animated;

  Vec2F32 scroll;
  Vec2F32 scroll_animated;

  RectF32 fixed_rect;

  // hampus: Per build info provided by builders
  UI_BoxFlags flags;
  String8 string;

  UI_CustomDrawFunction *custom_draw;
  void *custom_draw_user_data;

  U8 active_state;

  F32 active_t;
  F32 hot_t;

  F_GlyphRun *glyph_run;

  // hampus: Styling values
#define X(name_upper, name_lower, type) type name_lower;
  stack_values
#undef X
};

struct UI_ParentStackNode
{
  UI_ParentStackNode *next;
  UI_Box *box;
};

struct UI_Comm
{
  UI_Box *box;
  Vec2F32 rel_mouse;
  Vec2F32 drag_delta;
  Vec2F32 scroll;
  Side fling_side[Axis2_COUNT];
  B8 clicked : 1;
  B8 pressed : 1;
  B8 released : 1;
  B8 double_clicked : 1;
  B8 right_pressed : 1;
  B8 right_released : 1;
  B8 dragging : 1;
  B8 hovering : 1;
  B8 commit : 1;
  B8 long_pressed : 1;
};

//////////////////////////////
// NOTE(hampus): Stack types

struct UI_SeedNode
{
  UI_SeedNode *next;
  U64 seed;
};

#define X(name_upper, name_lower, type) \
  struct ui_##name_upper##Node          \
  {                                     \
    ui_##name_upper##Node *next;        \
    type val;                           \
    B32 auto_pop;                       \
  };
stack_values
#undef X
//////////////////////////////
// NOTE(hampus): Text action types

typedef U32 UI_TextActionFlags;
enum
{
  UI_TextActionFlag_WordScan = (1 << 0),
  UI_TextActionFlag_KeepMark = (1 << 1),
  UI_TextActionFlag_Delete = (1 << 2),
  UI_TextActionFlag_Copy = (1 << 3),
  UI_TextActionFlag_Paste = (1 << 4),
  UI_TextActionFlag_ZeroDeltaWithSelection = (1 << 5),
  UI_TextActionFlag_DeltaPicksSelectionSide = (1 << 6),
  UI_TextActionFlag_SelectAll = (1 << 7),
};

struct UI_TextAction
{
  UI_TextActionFlags flags;
  Vec2S64 delta;
  U32 codepoint;
};

struct UI_TextActionNode
{
  UI_TextActionNode *next;
  UI_TextActionNode *prev;
  UI_TextAction action;
};

struct UI_TextActionList
{
  UI_TextActionNode *first;
  UI_TextActionNode *last;
};

struct UI_TextOp
{
  S64 new_cursor;
  S64 new_mark;
  Vec2S64 range;
  String8 replace_string;
  String8 copy_string;
};

struct UI_TextEditState
{
  S64 cursor;
  S64 mark;
};

//////////////////////////////
// NOTE(hampus): Core state

struct UI_RendererState;

struct UI_State
{
  Arena *arena;
  StaticArray<Arena *, 2> frame_arenas;

  UI_RendererState *renderer;

  StaticArray<UI_Box *, 4096> box_map;
  UI_FreeBox *first_free_box;

  UI_Key active_key;
  UI_Key hot_key;
  UI_Key focus_key;

  UI_Key prev_active_key;
  UI_Key prev_hot_key;

  UI_ParentStackNode *parent_stack;
  UI_SeedNode *seed_stack;

  UI_Box *root;
  UI_Box *normal_root;
  UI_Box *ctx_menu_root;

  UI_Key ctx_menu_key;
  UI_Key ctx_menu_anchor_key;
  Vec2F32 anchor_offset;

  UI_Key next_ctx_menu_key;
  UI_Key next_ctx_menu_anchor_key;
  Vec2F32 next_anchor_offset;

  F_Tag default_icon_font_tag;
  F_Tag default_font_tag;

  U64 build_idx;

  F64 dt;
  OS_EventList *os_events;
  OS_Handle os_window;

  Vec2F32 mouse_pos;
  Vec2F32 prev_mouse_pos;

  B32 draw_debug_lines;

  StaticArray<String8, UI_Icon_COUNT> ui_icon_to_string_table;

#define X(name_upper, name_lower, type) ui_##name_upper##Node *name_lower##_stack;
  stack_values
#undef X
};

//////////////////////////////
// NOTE(hampus): Helpers

[[nodiscard]] static Arena *ui_frame_arena();
[[nodiscard]] static String8 ui_str8_from_icon(UI_Icon icon);

//////////////////////////////
// NOTE(hampus): Keying

[[nodiscard]] static UI_Key ui_key_zero();
[[nodiscard]] static UI_Key ui_key_from_string(U64 seed, String8 string);
[[nodiscard]] static U64 ui_hash_from_seed_string(U64 seed, String8 string);

//////////////////////////////
// NOTE(hampus): Text action

[[nodiscard]] static UI_TextAction ui_text_action_from_event(OS_Event *event);
[[nodiscard]] static UI_TextActionList ui_text_action_list_from_events(Arena *arena, OS_EventList *event_list);
[[nodiscard]] static UI_TextOp ui_text_of_from_state_and_action(Arena *arena, String8 edit_str, UI_TextEditState *state, UI_TextAction *action);

//////////////////////////////
// NOTE(hampus): Init

static void ui_init(String8 default_font_path, String8 default_icon_path);

//////////////////////////////
// NOTE(hampus): Begin/End

static void ui_begin_build(OS_Handle window, OS_EventList *os_events, F64 dt);
static void ui_end_build();

//////////////////////////////
// NOTE(hampus): Box implementation

[[nodiscard]] static UI_Box *ui_box_alloc();
static void ui_box_free(UI_Box *box);
[[nodiscard]] static B32 ui_box_is_hot(UI_Box *box);
[[nodiscard]] static B32 ui_box_is_active(UI_Box *box);
[[nodiscard]] static B32 ui_box_is_focus(UI_Box *box);
[[nodiscard]] static B32 ui_box_is_nil(UI_Box *box);
[[nodiscard]] static UI_Box *ui_box_from_key(UI_Key key);
static UI_Box *ui_box_make_from_key(UI_BoxFlags flags, UI_Key key);
[[nodiscard]] static String8 ui_get_display_part_from_string(String8 string);
[[nodiscard]] static String8 ui_get_hash_part_from_string(String8 string);
static UI_Box *ui_box_make(UI_BoxFlags flags, String8 string);
static UI_Box *ui_box_make(UI_BoxFlags flags, char *fmt, ...);
static UI_Box *ui_box_make(UI_BoxFlags flags);
static void ui_box_equip_display_string(UI_Box *box, String8 string);
static void ui_box_equip_display_string(UI_Box *box, char *fmt, ...);
static UI_Comm ui_comm_from_box__touch(UI_Box *box);
static UI_Comm ui_comm_from_box__mouse(UI_Box *box);

//////////////////////////////
// NOTE(hampus): Layouting

[[nodiscard]] static UI_Size ui_size_make(UI_SizeKind kind, F32 val, F32 strictness);
static void ui_solve_independent_sizes(UI_Box *root, Axis2 axis);
static void ui_solve_upward_dependent_sizes(UI_Box *root, Axis2 axis);
static void ui_solve_downward_dependent_sizes(UI_Box *root, Axis2 axis);
static void ui_solve_size_violations(UI_Box *root, Axis2 axis);
static void ui_calculate_final_rect(UI_Box *root, Axis2 axis);
static void ui_calculate_sizes(UI_Box *root);
static void ui_layout(UI_Box *root);

//////////////////////////////
// NOTE(hampus): Stack helpers

[[nodiscard]] static UI_Size ui_dp(F32 val, F32 strictness);
[[nodiscard]] static UI_Size ui_px(F32 val, F32 strictness);
[[nodiscard]] static UI_Size ui_text_content(F32 strictness);
[[nodiscard]] static UI_Size ui_pct(F32 val, F32 strictness);
[[nodiscard]] static UI_Size ui_children_sum(F32 strictness);
[[nodiscard]] static UI_Size ui_other_axis(F32 strictness);
[[nodiscard]] static UI_Size ui_fill();
[[nodiscard]] static UI_Size ui_em(F32 val, F32 strictness);

static void ui_push_parent(UI_Box *box);
static UI_Box *ui_pop_parent();
[[nodiscard]] static UI_Box *ui_top_parent();
static void ui_push_seed(U64 seed);
static void ui_pop_seed();
[[nodiscard]] static U64 ui_top_seed();
[[nodiscard]] static F32 ui_top_font_line_height();

static void ui_push_pref_size(Axis2 axis, UI_Size size);
static void ui_next_pref_size(Axis2 axis, UI_Size size);
static void ui_pop_pref_size(Axis2 axis);

static void ui_next_rect_color(Vec4F32 color);
static void ui_push_rect_color(Vec4F32 color);
static void ui_pop_rect_color();

static void ui_next_fixed_rect(RectF32 rect);
static void ui_push_fixed_rect(RectF32 rect);
static void ui_pop_fixed_rect();

//////////////////////////////
// NOTE(hampus): Macro wrappers

#if OS_ANDROID
#  define ui_comm_from_box(box) ui_comm_from_box__touch(box)
#else
#  define ui_comm_from_box(box) ui_comm_from_box__mouse(box)
#endif

#define ui_ctx_menu(key) defer_loop_checked(ui_ctx_menu_begin(key), ui_ctx_menu_end())

#define ui_seed(seed) defer_loop(ui_push_seed(seed), ui_pop_seed())
#define ui_parent(box) defer_loop(ui_push_parent(box), ui_pop_parent())

#define ui_size_from_axis(root, axis) ((UI_Size[]){root->pref_width, root->pref_height})[axis]

#define ui_rect_color(new_val) defer_loop(ui_push_rect_color(new_val), ui_pop_rect_color())
#define ui_border_color(new_val) defer_loop(ui_push_border_color(new_val), ui_pop_border_color())
#define ui_border_thickness(new_val) defer_loop(ui_push_border_thickness(new_val), ui_pop_border_thickness())
#define ui_corner_radius(new_val) defer_loop(ui_push_corner_radius(new_val), ui_pop_corner_radius())
#define ui_softness(new_val) defer_loop(ui_push_softness(new_val), ui_pop_softness())
#define ui_hover_cursor(new_val) defer_loop(ui_push_hover_cursor(new_val), ui_pop_hover_cursor())
#define ui_slice(new_val) defer_loop(ui_push_slice(new_val), ui_pop_slice())
#define ui_text_color(new_val) defer_loop(ui_push_text_color(new_val), ui_pop_text_color())
#define ui_rel_pos(new_val) defer_loop(ui_push_fixed_pos(new_val), ui_pop_rel_pos())
#define ui_child_layout_axis(new_val) defer_loop(ui_push_child_layout_axis(new_val), ui_pop_child_layout_axis())
#define ui_box_flags(new_val) defer_loop(ui_push_box_flags(new_val), ui_pop_box_flags())
#define ui_font_tag(new_val) defer_loop(ui_push_font_tag(new_val), ui_pop_font_tag())
#define ui_font_size(new_val) defer_loop(ui_push_font_size(new_val), ui_pop_font_size())
#define ui_text_padding(new_val) defer_loop(ui_push_text_padding(new_val), ui_pop_text_padding())
#define ui_text_align(new_val) defer_loop(ui_push_text_align(new_val), ui_pop_text_align())
#define ui_pref_width(new_val) defer_loop(ui_push_pref_width(new_val), ui_pop_pref_width())
#define ui_pref_height(new_val) defer_loop(ui_push_pref_height(new_val), ui_pop_pref_height())
#define ui_alpha(new_val) defer_loop(ui_push_alpha(new_val), ui_pop_alpha())
#define ui_pref_size(axis, new_val) defer_loop(ui_push_pref_size(axis, new_val), ui_pop_pref_size(axis))

// hampus: Extra macro wrappers

#define X(name_upper, name_lower, type)                         \
  static ui_##name_upper##Node *ui_push_##name_lower(type val); \
  static ui_##name_upper##Node *ui_next_##name_lower(type val); \
  static ui_##name_upper##Node *ui_pop_##name_lower();          \
  [[nodiscard]] static type ui_top_##name_lower();

stack_values
#undef X

//////////////////////////////
// NOTE(hampus): Globals

static UI_State *ui_state;
static read_only UI_Box ui_nil_box;

#endif // UI_CORE_H
