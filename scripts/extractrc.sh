#! /usr/bin/env perl

### TODO: other copyrights, license?
# Copyright (c) 2004 Richard Evans <rich@ridas.com>

sub usage
{
  warn <<"EOF";

extractrc [flags] filenames

This script extracts messages from designer (.ui) and XMLGUI (.rc) files and
writes on standard output (usually redirected to rc.cpp) the equivalent
i18n() calls so that xgettext can parse them.

--tag=name        : Also extract the tag name(s). Repeat the flag to specify 
                    multiple names: --tag=tag_one --tag=tag_two

--tag-group=group : Use a group of tags - uses 'default' if omitted.
                    Valid groups are: @{[TAG_GROUPS()]}

--context=name    : Give i18n calls a context name: i18nc("name", ...)
--lines           : Include source line numbers in comments (deprecated, it is switched on by default now)
--help|?          : Display this summary

EOF

  exit;
}

###########################################################################################

use strict;
use warnings;
use Getopt::Long;

use constant TAG_GROUP => 
{
  default => "[tT][eE][xX][tT]|title|string|whatsthis|tooltip|label",
  koffice => "Example|GroupName|Text|Comment|Syntax|TypeName",
  none    => "",
};

use constant TAG_GROUPS => join ", ", map "'$_'", sort keys %{&TAG_GROUP};


###########################################################################################
# Add options here as necessary - perldoc Getopt::Long for details on GetOptions

GetOptions ( "tag=s"       => \my @opt_extra_tags,
             "tag-group=s" => \my $opt_tag_group,
             "context=s"   => \my $opt_context,       # I18N context
             "lines"       => \my $opt_lines,
             "help|?"      => \&usage );

unless( @ARGV )
{
  warn "No filename specified";
  exit;
}

$opt_tag_group ||= "default";

die "Unknown tag group: '$opt_tag_group', should be one of " . TAG_GROUPS
    unless exists TAG_GROUP->{$opt_tag_group};

my $tags = TAG_GROUP->{$opt_tag_group};
my $extra_tags  = join "", map "|" . quotemeta, @opt_extra_tags;
my $text_string = qr/($tags$extra_tags)( [^>]*)?>/;    # Precompile regexp


###########################################################################################
#  Escape characters in string exactly like uic does.
sub escape_like_uic ($) {
    my $text = shift;

    $text =~ s/&lt;/</g;
    $text =~ s/&gt;/>/g;
    $text =~ s/&amp;/&/g;
    $text =~ s/&quot;/"/g;

    $text =~ s/\\/\\\\/g; # escape \
    $text =~ s/\"/\\\"/g; # escape "
    $text =~ s/\r//g; # remove CR (Carriage Return)
    $text =~ s/\n/\\n\"\n\"/g; # escape LF (Line Feed). uic also change the code line at a LF, we do not do that.

    return $text;
}

###########################################################################################
# Program start proper - NB $. is the current line number

for my $file_name ( @ARGV )
{
  my $fh;

  unless ( open $fh, "<", $file_name )
  {
    # warn "Failed to open: '$file_name': $!";
    next;
  }

  my $string          = "";
  my $in_text         = 0;
  my $start_line_no   = 0;
  my $in_skipped_prop = 0;
  my $tag = "";
  my $attr = "";
  my $context = "";

  while ( <$fh> )
  {
     last if $. == 1 and $_ !~ /^(?:<!DOCTYPE|<\?xml|<!--|<ui version=)/;

     chomp;

     $string .= "\n" . $_;
     
     # 'database', 'associations', 'populationText' and 'styleSheet' properties contain strings that shouldn't be translated

     if ( $in_skipped_prop == 0 and $string =~ /<property name=\"(?:database|associations|populationText|styleSheet)\"/ )
     {
       $in_skipped_prop = 1;
     }
     elsif ( $in_skipped_prop and $string =~ /<\/property/ )
     {
       $string          = "";
       $in_skipped_prop = 0;
     }

     $context = $opt_context unless $in_text;

     unless ( $in_skipped_prop or $in_text )
     {
       if ( ($tag, $attr) = $string =~ /<$text_string/o )
       {
         my ($attr_comment) = $attr =~ /\w*comment=\"([^\"]*)\"/ if $attr;
         $context = $attr_comment if $attr_comment;
         my ($attr_context) = $attr =~ /\w*context=\"([^\"]*)\"/ if $attr;
         $context = $attr_context if $attr_context;
         # It is unlikely that both attributes 'context' and 'comment'
         # will be present, but if so happens, 'context' has priority.

         $string        =~ s/^.*<$text_string//so;
         $in_text       =  1;
         $start_line_no =  $.;
       }
       else
       {
         $string = "";
       }
     }

     next unless $in_text;
     next unless $string =~ /<\/$text_string/o;

     my $text = $string;
     $text =~ s/<\/$text_string.*$//o;

     if ( $text cmp "" )
     {
       if ( not $context or $context ne "KDE::DoNotExtract" )
       {
         print "//i18n: tag $tag\n";
         print "//i18n: file $file_name line $.\n";
         print "// xgettext: no-c-format\n" if $text =~ /%/o;
         if ( $context )
         {
           $context = escape_like_uic($context);
           $text = escape_like_uic($text);
           print qq|i18nc("$context","$text");\n|;
         }
         else
         {
           $text = escape_like_uic($text);
           print  qq|i18n("$text");\n|;
         }
       }
       else
       {
         print "// Manually excluded message at $file_name line $.\n";
       }
     }
     else
     {
       print "// Skipped empty message at $file_name line $.\n";
     }

     $string  =~ s/^.*<\/$text_string//o;
     $in_text =  0;

     # Text can be multiline in .ui files (possibly), but we warn about it in XMLGUI .rc files.

     warn "there is <text> floating in: '$file_name'" if $. != $start_line_no and $file_name =~ /\.rc$/i;
  }

  close $fh or warn "Failed to close: '$file_name': $!";

  die "parsing error in $file_name" if $in_text;
}

