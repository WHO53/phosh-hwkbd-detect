#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include "proto/phoc-device-state-unstable-v1-client-protocol.h"

struct device_state {
    struct zphoc_device_state_v1 *device_state;
    uint32_t capabilities;
};

static void
handle_capabilities(void *data, struct zphoc_device_state_v1 *zphoc_device_state_v1,
                    uint32_t capabilities)
{
    struct device_state *state = data;
    state->capabilities = capabilities;

    if (capabilities & ZPHOC_DEVICE_STATE_V1_CAPABILITY_KEYBOARD) {
        printf("Hardware keyboard detected!\n");
    } else {
        printf("No hardware keyboard detected.\n");
    }
}

static const struct zphoc_device_state_v1_listener device_state_listener = {
    .capabilities = handle_capabilities,
};

static void
registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                       const char *interface, uint32_t version)
{
    struct device_state *state = data;
    if (strcmp(interface, zphoc_device_state_v1_interface.name) == 0) {
        state->device_state = wl_registry_bind(registry, name,
                                               &zphoc_device_state_v1_interface, 2);
        zphoc_device_state_v1_add_listener(state->device_state,
                                           &device_state_listener, state);
    }
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    // Handle removal if needed
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove
};

int main(int argc, char *argv[]) {
    struct wl_display *display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        return 1;
    }

    struct wl_registry *registry = wl_display_get_registry(display);
    struct device_state state = { 0 };

    wl_registry_add_listener(registry, &registry_listener, &state);

    wl_display_roundtrip(display);

    if (!state.device_state) {
        fprintf(stderr, "No phoc_device_state_v1 interface found\n");
        return 1;
    }

    // Event loop to continuously listen for device state changes
    while (wl_display_dispatch(display) != -1) {
        // Waiting for events, handle them as they come in
    }

    // Cleanup
    if (state.device_state)
        zphoc_device_state_v1_destroy(state.device_state);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);

    return 0;
}
