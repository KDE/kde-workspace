#!/usr/bin/perl -w
#
# Note: This file is taken, and modified, from gucharmap/gen-guch-unicode-tables.pl - svn revision 1040
#
# $Id$ 
#
# generates in the current directory:
#  - UnicodeBlocks.h
#  - unicode-names.h
#  - unicode-nameslist.h
#  - unicode-unihan.h
#  - UnicodeCategories.h
#  - UnicodeScripts.h
#
# usage: ./gen-guch-unicode-tables.pl UNICODE-VERSION DIRECTORY
# where DIRECTORY contains UnicodeData.txt Unihan.zip NamesList.txt Blocks.txt Scripts.txt
#

use strict;
use vars ('$UNZIP', '$ICONV');

# if these things aren't in your path you can put full paths to them here
$UNZIP = 'unzip';
$ICONV = 'iconv';

sub process_unicode_data_txt ($);
sub process_unihan_zip ($);
sub process_nameslist_txt ($);
sub process_blocks_txt ($);
sub process_scripts_txt ($);

$| = 1;  # flush stdout buffer

if (@ARGV != 2) 
{
    $0 =~ s@.*/@@;
    die <<EOF

Usage: $0 UNICODE-VERSION DIRECTORY

DIRECTORY should contain the following Unicode data files:
UnicodeData.txt Unihan.zip NamesList.txt Blocks.txt Scripts.txt

which can be found at http://www.unicode.org/Public/UNIDATA/

EOF
}

my ($unicodedata_txt, $unihan_zip, $nameslist_txt, $blocks_txt, $scripts_txt);

my $v = $ARGV[0];
my $d = $ARGV[1];
opendir (my $dir, $d) or die "Cannot open Unicode data dir $d: $!\n";
for my $f (readdir ($dir))
{
    $unicodedata_txt = "$d/$f" if ($f =~ /UnicodeData.*\.txt/);
#     $unihan_zip = "$d/$f" if ($f =~ /Unihan.*\.zip/);
#     $nameslist_txt = "$d/$f" if ($f =~ /NamesList.*\.txt/);
    $blocks_txt = "$d/$f" if ($f =~ /Blocks.*\.txt/);
    $scripts_txt = "$d/$f" if ($f =~ /Scripts.*\.txt/);
}

defined $unicodedata_txt or die "Did not find $d/UnicodeData.txt";
# defined $unihan_zip or die "Did not find $d/Unihan.zip";
# defined $nameslist_txt or die "Did not find $d/NamesList.txt";
defined $blocks_txt or die "Did not find $d/Blocks.txt";
defined $scripts_txt or die "Did not find $d/Scripts.txt";

process_unicode_data_txt ($unicodedata_txt);
# process_nameslist_txt ($nameslist_txt);
process_blocks_txt ($blocks_txt);
process_scripts_txt ($scripts_txt);
# process_unihan_zip ($unihan_zip);

exit;


#------------------------#

sub process_unicode_data_txt ($)
{
    my ($unicodedata_txt) = @_;

    # part 1: names

    open (my $unicodedata, $unicodedata_txt) or die;
#     open (my $out, "> unicode-names.h") or die;

    print "processing $unicodedata_txt...";
#
#     print $out "/* unicode-names.h */\n";
#     print $out "/* THIS IS A GENERATED FILE. CHANGES WILL BE OVERWRITTEN. */\n";
#     print $out "/* Generated by $0 */\n";
#     print $out "/* Generated from UCD version $v */\n\n";
#
#     print $out "#ifndef UNICODE_NAMES_H\n";
#     print $out "#define UNICODE_NAMES_H\n\n";
#
#     print $out "#include <glib/gunicode.h>\n\n";
#     print $out "#include \"gucharmap-intl.h\"\n\n";
#
#     my @unicode_pairs;
#     my %names;
#
#     while (my $line = <$unicodedata>)
#     {
#         chomp $line;
#         $line =~ /^([^;]+);([^;]+)/ or die;
#
#         my $hex = $1;
#         my $name = $2;
#
#         $names{$name} = 1;
#         push @unicode_pairs, [$hex, $name];
#     }
#
#     print $out "static const char unicode_names_strings[] = \\\n";
#
#     my $offset = 0;
#
#     foreach my $name (sort keys %names) {
# 	print $out "  \"$name\\0\"\n";
# 	$names{$name} = $offset;
# 	$offset += length($name) + 1;
#     }
#
#     undef $offset;
#
#     print $out ";\n";
#
#     print $out "typedef struct _UnicodeName UnicodeName;\n\n";
#
#     print $out "static const struct _UnicodeName\n";
#     print $out "{\n";
#     print $out "  gunichar index;\n";
#     print $out "  guint32 name_offset;\n";
#     print $out "} \n";
#     print $out "unicode_names[] =\n";
#     print $out "{\n";
#
#     my $first_line = 1;
#
#     foreach my $pair (@unicode_pairs) {
# 	if (!$first_line) {
# 	    print $out ",\n";
# 	} else {
# 	    $first_line = 0;
# 	}
#
# 	my ($hex, $name) = @{$pair};
# 	my $offset = $names{$name};
# 	print $out "  {0x$hex, $offset}";
#     }
#
#     print $out "\n};\n\n";
#
#     print $out <<EOT;
# static inline const char * unicode_name_get_name(const UnicodeName *entry)
# {
#   guint32 offset = entry->name_offset;
#   return unicode_names_strings + offset;
# }
#
# EOT
#
#     print $out "#endif  /* #ifndef UNICODE_NAMES_H */\n";
#
#     undef %names;
#     undef @unicode_pairs;
#
#     close ($unicodedata);
#     close ($out);

    # part 2: categories

    open ($unicodedata, $unicodedata_txt) or die;
    open (my $out, "> UnicodeCategories.h") or die;

    # Map general category code onto symbolic name.
    my %mappings =
    (
        # Normative.
        'Lu' => "UNICODE_UPPERCASE_LETTER",
        'Ll' => "UNICODE_LOWERCASE_LETTER",
        'Lt' => "UNICODE_TITLECASE_LETTER",
        'Mn' => "UNICODE_NON_SPACING_MARK",
        'Mc' => "UNICODE_COMBINING_MARK",
        'Me' => "UNICODE_ENCLOSING_MARK",
        'Nd' => "UNICODE_DECIMAL_NUMBER",
        'Nl' => "UNICODE_LETTER_NUMBER",
        'No' => "UNICODE_OTHER_NUMBER",
        'Zs' => "UNICODE_SPACE_SEPARATOR",
        'Zl' => "UNICODE_LINE_SEPARATOR",
        'Zp' => "UNICODE_PARAGRAPH_SEPARATOR",
        'Cc' => "UNICODE_CONTROL",
        'Cf' => "UNICODE_FORMAT",
        'Cs' => "UNICODE_SURROGATE",
        'Co' => "UNICODE_PRIVATE_USE",
        'Cn' => "UNICODE_UNASSIGNED",

        # Informative.
        'Lm' => "UNICODE_MODIFIER_LETTER",
        'Lo' => "UNICODE_OTHER_LETTER",
        'Pc' => "UNICODE_CONNECT_PUNCTUATION",
        'Pd' => "UNICODE_DASH_PUNCTUATION",
        'Ps' => "UNICODE_OPEN_PUNCTUATION",
        'Pe' => "UNICODE_CLOSE_PUNCTUATION",
        'Pi' => "UNICODE_INITIAL_PUNCTUATION",
        'Pf' => "UNICODE_FINAL_PUNCTUATION",
        'Po' => "UNICODE_OTHER_PUNCTUATION",
        'Sm' => "UNICODE_MATH_SYMBOL",
        'Sc' => "UNICODE_CURRENCY_SYMBOL",
        'Sk' => "UNICODE_MODIFIER_SYMBOL",
        'So' => "UNICODE_OTHER_SYMBOL"
    );

    # these shouldn't be -1
    my ($codepoint, $last_codepoint, $start_codepoint) = (-999, -999, -999);

    my ($category, $last_category) = ("FAKE1", "FAKE2");
    my ($started_range, $finished_range) = (undef, undef);

    print $out "/* UnicodeCategories.h */\n";
    print $out "/* THIS IS A GENERATED FILE. CHANGES WILL BE OVERWRITTEN. */\n";
    print $out "/* Generated by $0 */\n";
    print $out "/* Generated from UCD version $v */\n\n";

    print $out "#ifndef UNICODE_CATEGORIES_H\n";
    print $out "#define UNICODE_CATEGORIES_H\n\n";
    print $out "#include <QtCore/qglobal.h>\n\n";
    print $out "enum EUnicodeCategory\n";
    print $out "{\n";
    print $out "    UNICODE_UPPERCASE_LETTER,\n";
    print $out "    UNICODE_LOWERCASE_LETTER,\n";
    print $out "    UNICODE_TITLECASE_LETTER,\n";
    print $out "    UNICODE_NON_SPACING_MARK,\n";
    print $out "    UNICODE_COMBINING_MARK,\n";
    print $out "    UNICODE_ENCLOSING_MARK,\n";
    print $out "    UNICODE_DECIMAL_NUMBER,\n";
    print $out "    UNICODE_LETTER_NUMBER,\n";
    print $out "    UNICODE_OTHER_NUMBER,\n";
    print $out "    UNICODE_SPACE_SEPARATOR,\n";
    print $out "    UNICODE_LINE_SEPARATOR,\n";
    print $out "    UNICODE_PARAGRAPH_SEPARATOR,\n";
    print $out "    UNICODE_CONTROL,\n";
    print $out "    UNICODE_FORMAT,\n";
    print $out "    UNICODE_SURROGATE,\n";
    print $out "    UNICODE_PRIVATE_USE,\n";
    print $out "    UNICODE_UNASSIGNED,\n";
    print $out "    UNICODE_MODIFIER_LETTER,\n";
    print $out "    UNICODE_OTHER_LETTER,\n";
    print $out "    UNICODE_CONNECT_PUNCTUATION,\n";
    print $out "    UNICODE_DASH_PUNCTUATION,\n";
    print $out "    UNICODE_OPEN_PUNCTUATION,\n";
    print $out "    UNICODE_CLOSE_PUNCTUATION,\n";
    print $out "    UNICODE_INITIAL_PUNCTUATION,\n";
    print $out "    UNICODE_FINAL_PUNCTUATION,\n";
    print $out "    UNICODE_OTHER_PUNCTUATION,\n";
    print $out "    UNICODE_MATH_SYMBOL,\n";
    print $out "    UNICODE_CURRENCY_SYMBOL,\n";
    print $out "    UNICODE_MODIFIER_SYMBOL,\n";
    print $out "    UNICODE_OTHER_SYMBOL,\n";
    print $out "\n";
    print $out "    UNICODE_INVALID\n";
    print $out "};\n\n";
    print $out "struct TUnicodeCategory\n";
    print $out "{\n";
    print $out "    quint32 start;\n";
    print $out "    quint32 end;\n";
    print $out "    EUnicodeCategory category;\n";
    print $out "};\n\n";
    print $out "static const TUnicodeCategory constUnicodeCategoryList[] =\n";
    print $out "{\n";

    while (my $line = <$unicodedata>)
    {
        $line =~ /^([0-9A-F]*);([^;]*);([^;]*);/ or die;
        my $codepoint = hex ($1);
        my $name = $2;
        my $category = $mappings{$3};

        if ($finished_range 
            or ($category ne $last_category) 
            or (not $started_range and $codepoint != $last_codepoint + 1))
        {
            if ($last_codepoint >= 0) {
                printf $out ("    { 0x%4.4X, 0x%4.4X, \%s },\n", $start_codepoint, $last_codepoint, $last_category);
            } 

            $start_codepoint = $codepoint;
        }

        if ($name =~ /^<.*First>$/) {
            $started_range = 1;
            $finished_range = undef;
        }
        elsif ($name =~ /^<.*Last>$/) {
            $started_range = undef;
            $finished_range = 1;
        }
        elsif ($finished_range) {
            $finished_range = undef;
        }

        $last_codepoint = $codepoint;
        $last_category = $category;
    }
    printf $out ("    { 0x%4.4X, 0x%4.4X, \%s },\n", $start_codepoint, $last_codepoint, $last_category);
    printf $out "    { 0x0, 0x0, UNICODE_INVALID }\n";
    print $out "};\n\n";

    print $out "#endif\n";

    close ($out);
    print " done.\n";
}

#------------------------#

# XXX should do kFrequency too
sub process_unihan_zip ($)
{
    my ($unihan_zip) = @_;

    open (my $unihan, "$UNZIP -c $unihan_zip |") or die;
    open (my $out, "> unicode-unihan.h") or die;

    print "processing $unihan_zip";

    print $out "/* unicode-unihan.h */\n";
    print $out "/* THIS IS A GENERATED FILE. CHANGES WILL BE OVERWRITTEN. */\n";
    print $out "/* Generated by $0 */\n";
    print $out "/* Generated from UCD version $v */\n\n";

    print $out "#ifndef UNICODE_UNIHAN_H\n";
    print $out "#define UNICODE_UNIHAN_H\n\n";

    print $out "#include <glib/gunicode.h>\n\n";

    print $out "typedef struct _Unihan Unihan;\n\n";

    print $out "static const struct _Unihan\n";
    print $out "{\n";
    print $out "  gunichar index;\n";
    print $out "  gint32 kDefinition;\n";
    print $out "  gint32 kCantonese;\n";
    print $out "  gint32 kMandarin;\n";
    print $out "  gint32 kTang;\n";
    print $out "  gint32 kKorean;\n";
    print $out "  gint32 kJapaneseKun;\n";
    print $out "  gint32 kJapaneseOn;\n";
    print $out "} \n";
    print $out "unihan[] =\n";
    print $out "{\n";

    my @strings;
    my $offset = 0;

    my $wc = 0;
    my ($kDefinition, $kCantonese, $kMandarin, $kTang, $kKorean, $kJapaneseKun, $kJapaneseOn);

    my $i = 0;
    while (my $line = <$unihan>)
    {
        chomp $line;
        $line =~ /^U\+([0-9A-F]+)\s+([^\s]+)\s+(.+)$/ or next;

        my $new_wc = hex ($1);
        my $field = $2;

        my $value = $3;
        $value =~ s/\\/\\\\/g;
        $value =~ s/\"/\\"/g;

        if ($new_wc != $wc)
        {
            if (defined $kDefinition or defined $kCantonese or defined $kMandarin 
                or defined $kTang or defined $kKorean or defined $kJapaneseKun
                or defined $kJapaneseOn)
            {
                printf $out ("  { 0x%04X, \%d, \%d, \%d, \%d, \%d, \%d, \%d },\n",
                             $wc,
                             (defined($kDefinition) ? $kDefinition : -1),
                             (defined($kCantonese) ? $kCantonese: -1),
                             (defined($kMandarin) ? $kMandarin : -1),
                             (defined($kTang) ? $kTang : -1),
                             (defined($kKorean) ? $kKorean : -1),
                             (defined($kJapaneseKun) ? $kJapaneseKun : -1),
                             (defined($kJapaneseOn) ? $kJapaneseOn : -1));
            }

            $wc = $new_wc;

            undef $kDefinition;
            undef $kCantonese;
            undef $kMandarin;
            undef $kTang;
            undef $kKorean;
            undef $kJapaneseKun;
            undef $kJapaneseOn;
        }

        for my $f qw(kDefinition kCantonese kMandarin
                     kTang kKorean kJapaneseKun kJapaneseOn) {

            if ($field eq $f) {
	        push @strings, $value;
		my $last_offset = $offset;
		$offset += length($value) + 1;
		$value = $last_offset;
		last;
	    }
	}

        if ($field eq "kDefinition") {
            $kDefinition = $value;
        }
        elsif ($field eq "kCantonese") {
            $kCantonese = $value;
        }
        elsif ($field eq "kMandarin") {
            $kMandarin = $value;
        }
        elsif ($field eq "kTang") {
            $kTang = $value;
        }
        elsif ($field eq "kKorean") {
            $kKorean = $value;
        }
        elsif ($field eq "kJapaneseKun") {
            $kJapaneseKun = $value;
        }
        elsif ($field eq "kJapaneseOn") {
            $kJapaneseOn = $value;
        }

        if ($i++ % 32768 == 0) {
            print ".";
        }
    }

    print $out "};\n\n";

    print $out "static const char unihan_strings[] = \\\n";

    for my $s (@strings) {
	print $out "  \"$s\\0\"\n";
    }
    print $out ";\n\n";

    print $out "static const Unihan *_get_unihan (gunichar uc)\n;";

    for my $name qw(kDefinition kCantonese kMandarin
		    kTang kKorean kJapaneseKun kJapaneseOn) {
    print $out <<EOT;

static inline const char * unihan_get_$name (const Unihan *uh)
{
    gint32 offset = uh->$name;
    if (offset == -1)
      return NULL;
    return unihan_strings + offset;
}

G_CONST_RETURN gchar * 
gucharmap_get_unicode_$name (gunichar uc)
{
  const Unihan *uh = _get_unihan (uc);
  if (uh == NULL)
    return NULL;
  else
    return unihan_get_$name (uh);
}

EOT
    }

    print $out "#endif  /* #ifndef UNICODE_UNIHAN_H */\n";

    close ($unihan);
    close ($out);

    print " done.\n";
}

#------------------------#

# $nameslist_hash = 
# {
#     0x0027 => { '=' => { 
#                          'index'  => 30, 
#                          'values' => [ 'APOSTROPHE-QUOTE', 'APL quote' ]
#                        }
#                 '*' => {
#                          'index'  => 50,
#                          'values' => [ 'neutral (vertical) glyph with mixed usage',
#                                        '2019 is preferred for apostrophe',
#                                        'preferred characters in English for paired quotation marks are 2018 & 2019'
#                                      ]
#                         }
#                  # etc
#                }
#     # etc 
# };
# 
sub process_nameslist_txt ($)
{
    my ($nameslist_txt) = @_;

    open (my $nameslist, "$ICONV -f 'ISO8859-1' -t 'UTF-8' $nameslist_txt |") or die;

    print "processing $nameslist_txt...";

    my ($equal_i, $ex_i, $star_i, $pound_i, $colon_i) = (0, 0, 0, 0, 0);
    my $wc = 0;

    my $nameslist_hash;

    while (my $line = <$nameslist>)
    {
        chomp ($line);

        if ($line =~ /^@/)
        {
            next;
        }
        elsif ($line =~ /^([0-9A-F]+)/)
        {
            $wc = hex ($1);
        }
        elsif ($line =~ /^\s+=\s+(.+)$/)
        {
            my $value = $1;
            $value =~ s/\\/\\\\/g;
            $value =~ s/\"/\\"/g;

            if (not defined $nameslist_hash->{$wc}->{'='}->{'index'}) {
                $nameslist_hash->{$wc}->{'='}->{'index'} = $equal_i;
            }
            push (@{$nameslist_hash->{$wc}->{'='}->{'values'}}, $value);

            $equal_i++;
        }
        elsif ($line =~ /^\s+\*\s+(.+)$/)
        {
            my $value = $1;
            $value =~ s/\\/\\\\/g;
            $value =~ s/\"/\\"/g;

            if (not defined $nameslist_hash->{$wc}->{'*'}->{'index'}) {
                $nameslist_hash->{$wc}->{'*'}->{'index'} = $star_i;
            }
            push (@{$nameslist_hash->{$wc}->{'*'}->{'values'}}, $value);

            $star_i++;
        }
        elsif ($line =~ /^\s+#\s+(.+)$/)
        {
            my $value = $1;
            $value =~ s/\\/\\\\/g;
            $value =~ s/\"/\\"/g;

            if (not defined $nameslist_hash->{$wc}->{'#'}->{'index'}) {
                $nameslist_hash->{$wc}->{'#'}->{'index'} = $pound_i;
            }
            push (@{$nameslist_hash->{$wc}->{'#'}->{'values'}}, $value);

            $pound_i++;
        }
        elsif ($line =~ /^\s+:\s+(.+)$/)
        {
            my $value = $1;
            $value =~ s/\\/\\\\/g;
            $value =~ s/\"/\\"/g;

            if (not defined $nameslist_hash->{$wc}->{':'}->{'index'}) {
                $nameslist_hash->{$wc}->{':'}->{'index'} = $colon_i;
            }
            push (@{$nameslist_hash->{$wc}->{':'}->{'values'}}, $value);

            $colon_i++;
        }
        elsif ($line =~ /^\s+x\s+.*([0-9A-F]{4,6})\)$/)  # this one is different
        {
            my $value = hex ($1);

            if (not defined $nameslist_hash->{$wc}->{'x'}->{'index'}) {
                $nameslist_hash->{$wc}->{'x'}->{'index'} = $ex_i;
            }
            push (@{$nameslist_hash->{$wc}->{'x'}->{'values'}}, $value);

            $ex_i++;
        }
    }

    close ($nameslist);

    open (my $out, "> unicode-nameslist.h") or die;

    print $out "/* unicode-nameslist.h */\n";
    print $out "/* THIS IS A GENERATED FILE. CHANGES WILL BE OVERWRITTEN. */\n";
    print $out "/* Generated by $0 */\n";
    print $out "/* Generated from UCD version $v */\n\n";

    print $out "#ifndef UNICODE_NAMESLIST_H\n";
    print $out "#define UNICODE_NAMESLIST_H\n\n";

    print $out "#include <glib/gunicode.h>\n\n";

    print $out "typedef struct _UnicharString UnicharString;\n";
    print $out "typedef struct _UnicharUnichar UnicharUnichar;\n";
    print $out "typedef struct _NamesList NamesList;\n\n";

    print $out "struct _UnicharString\n";
    print $out "{\n";
    print $out "  gunichar index;\n";
    print $out "  const gchar *value;\n";
    print $out "}; \n\n";

    print $out "struct _UnicharUnichar\n";
    print $out "{\n";
    print $out "  gunichar index;\n";
    print $out "  gunichar value;\n";
    print $out "}; \n\n";

    print $out "struct _NamesList\n";
    print $out "{\n";
    print $out "  gunichar index;\n";
    print $out "  gint equals_index;  /* -1 means */\n";
    print $out "  gint stars_index;   /* this character */\n";
    print $out "  gint exes_index;    /* doesn't */\n";
    print $out "  gint pounds_index;  /* have any */\n";
    print $out "  gint colons_index;\n";
    print $out "};\n\n";

    print $out "static const UnicharString names_list_equals[] = \n";
    print $out "{\n";
    for $wc (sort {$a <=> $b} keys %{$nameslist_hash})
    {
        next if not exists $nameslist_hash->{$wc}->{'='};
        for my $value (@{$nameslist_hash->{$wc}->{'='}->{'values'}}) {
            printf $out (qq/  { 0x%04X, "\%s" },\n/, $wc, $value);
        }
    }
    print $out "  { (gunichar)(-1), 0 }\n";
    print $out "};\n\n";

    print $out "static const UnicharString names_list_stars[] = \n";
    print $out "{\n";
    for $wc (sort {$a <=> $b} keys %{$nameslist_hash})
    {
        next if not exists $nameslist_hash->{$wc}->{'*'};
        for my $value (@{$nameslist_hash->{$wc}->{'*'}->{'values'}}) {
            printf $out (qq/  { 0x%04X, "\%s" },\n/, $wc, $value);
        }
    }
    print $out "  { (gunichar)(-1), 0 }\n";
    print $out "};\n\n";

    print $out "static const UnicharString names_list_pounds[] = \n";
    print $out "{\n";
    for $wc (sort {$a <=> $b} keys %{$nameslist_hash})
    {
        next if not exists $nameslist_hash->{$wc}->{'#'};
        for my $value (@{$nameslist_hash->{$wc}->{'#'}->{'values'}}) {
            printf $out (qq/  { 0x%04X, "\%s" },\n/, $wc, $value);
        }
    }
    print $out "  { (gunichar)(-1), 0 }\n";
    print $out "};\n\n";

    print $out "static const UnicharUnichar names_list_exes[] = \n";
    print $out "{\n";
    for $wc (sort {$a <=> $b} keys %{$nameslist_hash})
    {
        next if not exists $nameslist_hash->{$wc}->{'x'};
        for my $value (@{$nameslist_hash->{$wc}->{'x'}->{'values'}}) {
            printf $out (qq/  { 0x%04X, 0x%04X },\n/, $wc, $value);
        }
    }
    print $out "  { (gunichar)(-1), 0 }\n";
    print $out "};\n\n";

    print $out "static const UnicharString names_list_colons[] = \n";
    print $out "{\n";
    for $wc (sort {$a <=> $b} keys %{$nameslist_hash})
    {
        next if not exists $nameslist_hash->{$wc}->{':'};
        for my $value (@{$nameslist_hash->{$wc}->{':'}->{'values'}}) {
            printf $out (qq/  { 0x%04X, "\%s" },\n/, $wc, $value);
        }
    }
    print $out "  { (gunichar)(-1), 0 }\n";
    print $out "};\n\n";

    print $out "static const NamesList names_list[] =\n";
    print $out "{\n";
    for $wc (sort {$a <=> $b} keys %{$nameslist_hash})
    {
        my $eq    = exists $nameslist_hash->{$wc}->{'='}->{'index'} ? $nameslist_hash->{$wc}->{'='}->{'index'} : -1;
        my $star  = exists $nameslist_hash->{$wc}->{'*'}->{'index'} ? $nameslist_hash->{$wc}->{'*'}->{'index'} : -1;
        my $ex    = exists $nameslist_hash->{$wc}->{'x'}->{'index'} ? $nameslist_hash->{$wc}->{'x'}->{'index'} : -1;
        my $pound = exists $nameslist_hash->{$wc}->{'#'}->{'index'} ? $nameslist_hash->{$wc}->{'#'}->{'index'} : -1;
        my $colon = exists $nameslist_hash->{$wc}->{':'}->{'index'} ? $nameslist_hash->{$wc}->{':'}->{'index'} : -1;

        printf $out ("  { 0x%04X, \%d, \%d, \%d, \%d, \%d },\n", $wc, $eq, $star, $ex, $pound, $colon);
    }
    print $out "};\n\n";

    print $out "#endif  /* #ifndef UNICODE_NAMESLIST_H */\n";

    close ($out);

    print " done.\n";
}

#------------------------#

sub process_blocks_txt ($)
{
    my ($blocks_txt) = @_;

    open (my $blocks, $blocks_txt) or die;
    open (my $out, "> UnicodeBlocks.h") or die;

    print "processing $blocks_txt...";

    print $out "/* UnicodeBlocks.h */\n";
    print $out "/* THIS IS A GENERATED FILE. CHANGES WILL BE OVERWRITTEN. */\n";
    print $out "/* Generated by $0 */\n";
    print $out "/* Generated from UCD version $v */\n\n";

    print $out "#ifndef __UNICODE_BLOCKS_H__\n";
    print $out "#define __UNICODE_BLOCKS_H__\n\n";

    print $out "#include <QtCore/qglobal.h>\n";
    print $out "#include <klocalizedstring.h>\n\n";

    print $out "struct TUnicodeBlock\n";
    print $out "{\n";
    print $out "    quint32    start,\n";
    print $out "               end;\n";
    print $out "    const char *blockName;\n";
    print $out "};\n\n";
    print $out "static const struct TUnicodeBlock constUnicodeBlocks[] =\n";
    print $out "{\n";
    while (my $line = <$blocks>)
    {
        $line =~ /^([0-9A-F]+)\.\.([0-9A-F]+); (.+)$/ or next;
        print $out qq/    { 0x$1, 0x$2, I18N_NOOP("$3") },\n/;
    }
    print $out "    { 0x0, 0x0, NULL }\n";
    print $out "};\n\n";

    print $out "#endif\n\n";

    close ($blocks);
    close ($out);

    print " done.\n";
}

#------------------------#

sub process_scripts_txt ($)
{
    my ($scripts_txt) = @_;

    my %script_hash;
    my %scripts;

    open (my $scripts, $scripts_txt) or die;
    open (my $out, "> UnicodeScripts.h") or die;

    print "processing $scripts_txt...";

    while (my $line = <$scripts>)
    {
        my ($start, $end, $raw_script);

        if ($line =~ /^([0-9A-F]+)\.\.([0-9A-F]+)\s+;\s+(\S+)/)
        {
            $start = hex ($1);
            $end = hex ($2);
            $raw_script = $3;
        }
        elsif ($line =~ /^([0-9A-F]+)\s+;\s+(\S+)/)
        {
            $start = hex ($1);
            $end = $start;
            $raw_script = $2;
        }
        else 
        {
            next;
        }

        my $script = $raw_script;
        $script =~ tr/_/ /;
        $script =~ s/(\w+)/\u\L$1/g;

        $script_hash{$start} = { 'end' => $end, 'script' => $script };
        $scripts{$script} = 1;
    }

    close ($scripts);

    # Adds Common to make sure works with UCD <= 4.0.0
    $scripts{"Common"} = 1; 

    print $out "/* UnicodeScripts.h */\n";
    print $out "/* THIS IS A GENERATED FILE. CHANGES WILL BE OVERWRITTEN. */\n";
    print $out "/* Generated by $0 */\n";
    print $out "/* Generated from UCD version $v */\n\n";

    print $out "#ifndef __UNICODE_SCRIPTS_H__\n";
    print $out "#define __UNICODE_SCRIPTS_H__\n\n";

    print $out "#include <QtCore/qglobal.h>\n";
    print $out "#include <klocalizedstring.h>\n\n";

    print $out "static const char *constUnicodeScriptList[] =\n";
    print $out "{\n";
    my $i = 0;
    for my $script (sort keys %scripts)
    {
        $scripts{$script} = $i;
        print $out qq/    I18N_NOOP("$script"),\n/;
        $i++;
    }
    print $out "    NULL\n";
    print $out "};\n\n";

    print $out "struct TUnicodeScript\n";
    print $out "{\n";
    print $out "    quint32 start,\n";
    print $out "            end;\n";
    print $out "    int     scriptIndex;   /* index into constUnicodeScriptList */\n";
    print $out "};\n\n";
    print $out "static const TUnicodeScript constUnicodeScripts[] =\n";
    print $out "{\n";
    for my $start (sort { $a <=> $b } keys %script_hash) 
    {
        printf $out (qq/    { 0x%04X, 0x%04X, \%2d },\n/, 
                     $start, $script_hash{$start}->{'end'}, $scripts{$script_hash{$start}->{'script'}});
    }
    printf $out "    { 0x0, 0x0, -1 }\n";
    print $out "};\n\n";

    print $out "#endif\n\n";

    close ($out);
    print " done.\n";
}
