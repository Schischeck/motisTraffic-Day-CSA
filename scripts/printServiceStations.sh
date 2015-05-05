grep -A1 -B2 "_$2|" $1.Services.txt | \
  grep '^l[0-9]*$' | \
  awk -F "l" '{ print $2; }' | \
  xargs -I {} grep "^{}%" $1.Stations.txt
