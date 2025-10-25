#! /bin/sh

# Small helper script to fetch the Unicode Character Database files

VER=17.0.0

cd "$(dirname "$0")"
pwd

rm -rf Unicode.tables/
mkdir Unicode.tables

fetch_file()
{
  url="$1"
  i="$2"

  echo "=== Downloading $i ==="
  # Download each file with curl and place into the Unicode.tables folder
  # Reject the download if there is an HTTP error
  if ! curl --fail -o Unicode.tables/$i -L "$url"; then
    echo "Error downloading $i"
    rm -f Unicode.tables/$i
  fi
}

for i in BidiMirroring.txt \
         CaseFolding.txt \
         DerivedCoreProperties.txt \
         PropertyAliases.txt \
         PropertyValueAliases.txt \
         PropList.txt \
         ScriptExtensions.txt \
         Scripts.txt \
         UnicodeData.txt \
         ; do
  fetch_file "https://www.unicode.org/Public/$VER/ucd/$i" "$i"
done

for i in DerivedBidiClass.txt \
         DerivedGeneralCategory.txt \
         ; do
  fetch_file "https://www.unicode.org/Public/$VER/ucd/extracted/$i" "$i"
done

for i in GraphemeBreakProperty.txt \
         ; do
  fetch_file "https://www.unicode.org/Public/$VER/ucd/auxiliary/$i" "$i"
done

for i in emoji-data.txt \
         ; do
  fetch_file "https://www.unicode.org/Public/$VER/ucd/emoji/$i" "$i"
done
