cat > ~/Workspaces/qtcreator/bp-sose-2015-td-server-build/schedule/test.Classes.txt <<'EOF'
10
0 4 ICE THA TGV RJ
1 4 EC IC EX D
2 3 CNL EN AZ
3 8 IRE REX RE IR X DPX E Sp
4 5 DPN R DPF RB Os
5 2 S SB
6 2 U STB
7 1 STR
8 1 Bus
9 11 Flug Schiff ZahnR Schw-B Fähre KAT EZ ALT AST RFB RT
EOF

motis/prep/motis-prep --dataset ~/Workspaces/qtcreator/bp-sose-2015-td-server-build/schedule/test --services_to_routes --serialize_graph