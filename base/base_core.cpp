static OperatingSystem
os_from_context()
{
  OperatingSystem result = OperatingSystem_Null;
#if OS_WINDOWS
  result = OperatingSystem_Windows;
#elif OS_LINUX
  result = OperatingSystem_Linux;
#elif OS_MAC
  result = OperatingSystem_Mac;
#endif
  return result;
}

static Architecture
arch_from_context()
{
  Architecture result = Architecture_Null;
#if ARCH_X64
  result = Architecture_X64;
#elif ARCH_X86
  result = Architecture_X86;
#elif ARCH_ARM
  result = Architecture_ARM;
#elif ARCH_ARM64
  result = Architecture_ARM64;
#endif
  return result;
}

static DateTime
build_date_from_context()
{
  DateTime result = {};
  TempArena scratch = GetScratch(0, 0);

  // NOTE(hampus): Date is in the format "M D Y"
  // Example: "Dec 15 2023"
  String8 date = str8_cstr((char *)__DATE__);

  // NOTE(hampus): Time is in the format "HH:MM:SS"
  // Example: "11:53:37"
  String8 time = str8_cstr((char *)__TIME__);

  {
    String8List date_list = str8_split_by_codepoints(scratch.arena, date, Str8Lit(' '));

    String8Node *node = date_list.first;
    String8 month = node->v;

    ForEachEnumVal(Month, m)
    {
      if(str8_match(month, string_from_month(m)))
      {
        result.month = (U8)m;
      }
    }

    date = str8_skip(date, 4);

    U64 len = u8_from_str8(date, &result.day);
    date = str8_skip(date, len + 1);
    len = s16_from_str8(date, &result.year);
  }

  {
    U64 len = u8_from_str8(time, &result.hour);
    time = str8_skip(time, len + 1);
    len = u8_from_str8(time, &result.minute);
    time = str8_skip(time, len + 1);
    u8_from_str8(time, &result.second);
  }
  return result;
}

static String8
string_from_os(OperatingSystem os)
{
  String8 result = Str8Lit("");
  switch(os)
  {
    case OperatingSystem_Null:
    {
      result = Str8Lit("Null");
    }
    break;
    case OperatingSystem_Windows:
    {
      result = Str8Lit("Windows");
    }
    break;
    case OperatingSystem_Linux:
    {
      result = Str8Lit("Linux");
    }
    break;
    case OperatingSystem_Mac:
    {
      result = Str8Lit("Mac");
    }
    break;
      InvalidCase;
  }
  return result;
}

static String8
string_from_arch(Architecture arc)
{
  String8 result = Str8Lit("");
  switch(arc)
  {
    case Architecture_Null:
    {
      result = Str8Lit("Null");
    }
    break;
    case Architecture_X64:
    {
      result = Str8Lit("x64");
    }
    break;
    case Architecture_X86:
    {
      result = Str8Lit("x86");
    }
    break;
    case Architecture_ARM:
    {
      result = Str8Lit("ARM");
    }
    break;
    case Architecture_ARM64:
    {
      result = Str8Lit("ARM64");
    }
    break;
      InvalidCase;
  }

  return result;
}

static String8
string_from_day_of_week(DayOfWeek day)
{
  String8 result = Str8Lit("");
  switch(day)
  {
    case DayOfWeek_Monday:
    {
      result = Str8Lit("Monday");
    }
    break;
    case DayOfWeek_Tuesday:
    {
      result = Str8Lit("Tuesday");
    }
    break;
    case DayOfWeek_Wednesday:
    {
      result = Str8Lit("Wednesday");
    }
    break;
    case DayOfWeek_Thursday:
    {
      result = Str8Lit("Thursday");
    }
    break;
    case DayOfWeek_Friday:
    {
      result = Str8Lit("Friday");
    }
    break;
    case DayOfWeek_Saturday:
    {
      result = Str8Lit("Saturday");
    }
    break;
    case DayOfWeek_Sunday:
    {
      result = Str8Lit("Sunday");
    }
    break;
      InvalidCase;
  }
  return result;
}

static String8
string_from_month(Month month)
{
  String8 result = Str8Lit("");
  switch(month)
  {
    case Month_Jan:
    {
      result = Str8Lit("Jan");
    }
    break;
    case Month_Feb:
    {
      result = Str8Lit("Feb");
    }
    break;
    case Month_Mar:
    {
      result = Str8Lit("Mar");
    }
    break;
    case Month_Apr:
    {
      result = Str8Lit("Apr");
    }
    break;
    case Month_May:
    {
      result = Str8Lit("May");
    }
    break;
    case Month_Jun:
    {
      result = Str8Lit("Jun");
    }
    break;
    case Month_Jul:
    {
      result = Str8Lit("Jul");
    }
    break;
    case Month_Aug:
    {
      result = Str8Lit("Aug");
    }
    break;
    case Month_Sep:
    {
      result = Str8Lit("Sep");
    }
    break;
    case Month_Oct:
    {
      result = Str8Lit("Oct");
    }
    break;
    case Month_Nov:
    {
      result = Str8Lit("Nov");
    }
    break;
    case Month_Dec:
    {
      result = Str8Lit("Dec");
    }
    break;
      InvalidCase;
  }
  return result;
}

static DenseTime
dense_time_from_date_time(DateTime date_time)
{
  DenseTime result = {};
  result.time += safe_u32_from_s32((S32)date_time.year + 0x8000);
  result.time *= 12;
  result.time += date_time.month;
  result.time *= 31;
  result.time += date_time.day;
  result.time *= 24;
  result.time += date_time.hour;
  result.time *= 60;
  result.time += date_time.minute;
  result.time *= 61;
  result.time += date_time.second;
  result.time *= 1000;
  result.time += date_time.millisecond;
  return result;
}

static DateTime
date_time_from_dense_time(DenseTime dense_time)
{
  DateTime result = {};
  result.millisecond = (U16)(dense_time.time % 1000);
  dense_time.time /= 1000;
  result.second = (U8)(dense_time.time % 61);
  dense_time.time /= 61;
  result.minute = (U8)(dense_time.time % 60);
  dense_time.time /= 60;
  result.hour = (U8)(dense_time.time % 24);
  dense_time.time /= 24;
  result.day = (U8)(dense_time.time % 31);
  dense_time.time /= 31;
  result.month = (U8)(dense_time.time % 12);
  dense_time.time /= 12;
  result.year = (S16)((S32)dense_time.time - 0x8000);
  return result;
}

static B32
date_match(Date a, Date b)
{
  B32 result = a.year == b.year && a.month == b.month && a.day == b.day;
  return result;
}

static MemorySize
memory_size_from_bytes(U64 bytes)
{
  String8 units[] =
  {
   Str8Lit("KB"),
   Str8Lit("MB"),
   Str8Lit("GB"),
   Str8Lit("TB"),
  };

  MemorySize result;
  result.amount = (F64)bytes;
  result.unit = (U8 *)"B";
  result.unit_length = 1;
  for(U64 i = 0; i < ArrayCount(units) && result.amount > 2048.0f; ++i)
  {
    result.amount /= 1024.0f;
    result.unit = units[i].data;
    result.unit_length = units[i].size;
  }

  return result;
}