/*
 * Copyright 2016 Google Inc.
 * author: Edward Hervey <bilboed@bilboed.com>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <locale.h>

#include <stdlib.h>
#include <glib.h>
#include <gst/gst.h>

GST_PLUGIN_STATIC_DECLARE(coreelements);
GST_PLUGIN_STATIC_DECLARE(typefindfunctions);
GST_PLUGIN_STATIC_DECLARE(app);

/* push-based typefind fuzzing target
 *
 * This application can be compiled with libFuzzer to simulate
 * a push-based typefind execution.
 *
 * To reproduce the failing behaviour, use:
 * $ gst-launch-1.0 pushfile:///.. ! typefind ! fakesink
 *
 * The goal is to cover typefind code and implementation.
 *
 **/

static void
appsrc_configuration (GstDiscoverer *dc, GstElement *source, gpointer data)
{
  GstBuffer *buf;
  GstFlowReturn ret;
  
  /* Create buffer from fuzztesting_data which shouldn't be freed */
}

int LLVMFuzzerTestOneInput(const guint8 *data, size_t size)
{
  GError *err = NULL;
  static gboolean initialized = 0;
  GstElement *pipeline, *source, *typefind, *fakesink;
  GstBuffer *buf;
  GstFlowReturn flowret;

  if (!initialized) {
    /* We want critical warnings to assert so we can fix them
     * But somehow it's not causing oss-fuzz to crash ... */
    g_setenv ("G_DEBUG", "fatal-criticals", TRUE);

    /* Only initialize and register plugins once */
    gst_init (NULL, NULL);
    
    GST_PLUGIN_STATIC_REGISTER(coreelements);
    GST_PLUGIN_STATIC_REGISTER(typefindfunctions);
    GST_PLUGIN_STATIC_REGISTER(app);
  }
  
  /* Create the pipeline */
  source = gst_element_factory_make ("appsrc", "source");
  typefind = gst_element_factory_make ("typefind", "typefind");
  fakesink = gst_element_factory_make ("fakesink", "fakesink");

  gst_bin_add_many (GST_BIN (pipeline), source, typefind, fakesink, NULL);
  gst_element_link_many (source, typefind, fakesink, NULL);

  /* Set pipeline to READY so we can provide data to appsrc */
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_READY);
  buf = gst_buffer_new_wrapped_full (0, (gpointer) data, size,
				     0, size, NULL, NULL);
  g_object_set (G_OBJECT (source), "size", size, NULL);
  g_signal_emit_by_name (G_OBJECT(source), "push-buffer", buf, &ret);
  gst_buffer_unref (buf);
  
  /* Set pipeline to PAUSED and wait (typefind will either fail or succeed) */
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PAUSED);

  /* wait until state change either completes or fails */
  sret = gst_element_get_state (GST_ELEMENT (pipeline), &state, NULL, -1);

  /* Go back to NULL */
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);

  /* And release the pipeline */
  gst_object_unref (pipeline);
  
  return 0;
 }
 