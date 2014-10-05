/*  libloadorder

A library for reading and writing the load order of plugin files for
TES III: Morrowind, TES IV: Oblivion, TES V: Skyrim, Fallout 3 and
Fallout: New Vegas.

Copyright (C) 2012    WrinklyNinja

This file is part of libloadorder.

libloadorder is free software: you can redistribute
it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

libloadorder is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libloadorder.  If not, see
<http://www.gnu.org/licenses/>.
*/

#ifndef __LIBLO_TEST_API_LOAD_ORDER__
#define __LIBLO_TEST_API_LOAD_ORDER__

#include "tests/fixtures.h"

TEST_F(OblivionOperationsTest, GetLoadOrderMethod) {
    unsigned int method;
    EXPECT_EQ(LIBLO_OK, lo_get_load_order_method(gh, &method));
    EXPECT_EQ(LIBLO_METHOD_TIMESTAMP, method);

    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_get_load_order_method(NULL, NULL));
    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_get_load_order_method(gh, NULL));
    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_get_load_order_method(NULL, &method));
}

TEST_F(SkyrimOperationsTest, GetLoadOrderMethod) {
    unsigned int method;
    EXPECT_EQ(LIBLO_OK, lo_get_load_order_method(gh, &method));
    EXPECT_EQ(LIBLO_METHOD_TEXTFILE, method);

    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_get_load_order_method(NULL, NULL));
    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_get_load_order_method(gh, NULL));
    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_get_load_order_method(NULL, &method));
}

TEST_F(OblivionOperationsTest, SetLoadOrder) {
    // Can't redistribute Oblivion.esm, but Nehrim.esm can be,
    // so use that for testing.
    char * plugins[] = {
        "Blank.esm"
    };
    size_t pluginsNum = 1;

    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_set_load_order(gh, NULL, pluginsNum));
    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_set_load_order(gh, NULL, 0));

    // Test trying to set load order with non-Oblivion.esm without
    // first setting the game master.
    EXPECT_EQ(LIBLO_OK, lo_set_load_order(gh, plugins, 0));
    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_set_load_order(gh, plugins, pluginsNum));

    // Now set game master and try again.
    ASSERT_EQ(LIBLO_OK, lo_set_game_master(gh, "Blank.esm"));
    EXPECT_EQ(LIBLO_OK, lo_set_load_order(gh, plugins, pluginsNum));

    // Now test with more than one plugin.
    char * plugins2[] = {
        "Blank.esm",
        "Blank.esp"
    };
    pluginsNum = 2;
    EXPECT_EQ(LIBLO_OK, lo_set_load_order(gh, plugins2, pluginsNum));

    char * plugins3[] = {
        "Blank.esm",
        "Blank.esp.missing"
    };
    EXPECT_EQ(LIBLO_ERROR_FILE_NOT_FOUND, lo_set_load_order(gh, plugins3, pluginsNum));
}

TEST_F(OblivionOperationsTest, GetLoadOrder) {
    char ** plugins;
    size_t pluginsNum;
    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_get_load_order(gh, NULL, &pluginsNum));
    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_get_load_order(gh, &plugins, NULL));
    EXPECT_EQ(LIBLO_ERROR_INVALID_ARGS, lo_get_load_order(gh, NULL, NULL));

    EXPECT_EQ(LIBLO_OK, lo_get_load_order(gh, &plugins, &pluginsNum));
}

TEST_F(SkyrimOperationsTest, GetLoadOrder) {
    // Test that ghosted plugins get put into loadorder.txt correctly.

    // Set load order to ensure that test ghosted plugin is loaded early.
    char * plugins[] = {
        "Skyrim.esm",
        "Blank.esm",
        "Blank - Master Dependent.esm"
    };
    size_t pluginsNum = 1;
    ASSERT_EQ(LIBLO_OK, lo_set_load_order(gh, plugins, pluginsNum));

    // Now get load order.
    std::vector<std::string> actualLines;
    std::string content;
    ASSERT_TRUE(boost::filesystem::exists(localPath / "loadorder.txt"));
    liblo::fileToBuffer(localPath / "loadorder.txt", content);
    boost::split(actualLines, content, [](char c) {
        return c == '\n';
    });

    boost::filesystem::copy_file(localPath / "loadorder.txt", localPath / "loadorder.txt.copy");

    EXPECT_EQ("Blank - Master Dependent.esm", actualLines[2]);
}

TEST_F(OblivionOperationsTest, SetPluginPosition) {
    // First ensure than the game master comes first.
    char * plugins[] = {
        "Blank.esm"
    };
    size_t pluginsNum = 1;
    ASSERT_EQ(LIBLO_OK, lo_set_game_master(gh, "Blank.esm"));
    ASSERT_EQ(LIBLO_OK, lo_set_load_order(gh, plugins, pluginsNum));

    // Load filter patch last.
    EXPECT_EQ(LIBLO_OK, lo_set_plugin_position(gh, "Blank - Plugin Dependent.esp", 100));
}

TEST_F(OblivionOperationsTest, GetPluginPosition) {
    // First ensure than the game master comes first.
    char * plugins[] = {
        "Blank.esm"
    };
    size_t pluginsNum = 1;
    ASSERT_EQ(LIBLO_OK, lo_set_game_master(gh, "Blank.esm"));
    ASSERT_EQ(LIBLO_OK, lo_set_load_order(gh, plugins, pluginsNum));

    size_t pos;
    EXPECT_EQ(LIBLO_OK, lo_get_plugin_position(gh, "Blank.esm", &pos));
    EXPECT_EQ(0, pos);
}

TEST_F(OblivionOperationsTest, GetIndexedPlugin) {
    // First ensure than the game master comes first.
    char * plugins[] = {
        "Blank.esm"
    };
    size_t pluginsNum = 1;
    ASSERT_EQ(LIBLO_OK, lo_set_game_master(gh, "Blank.esm"));
    ASSERT_EQ(LIBLO_OK, lo_set_load_order(gh, plugins, pluginsNum));

    char * plugin;
    EXPECT_EQ(LIBLO_OK, lo_get_indexed_plugin(gh, 0, &plugin));
    EXPECT_STREQ("Blank.esm", plugin);
}

#endif