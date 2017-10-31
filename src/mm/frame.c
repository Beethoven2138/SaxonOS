#include <mm/frame.h>

void terminal_printhex32(uint32_t);

void init_frames(uint32_t end_kernel, uint32_t mem_size)
{
	const uint32_t first_free_frame = (end_kernel & 0x00FFF000) + 0x1000;
        nframes = mem_size / 4;

	memset(frame_map, 0, nframes/8);
	
	for (int i = 0; i < first_free_frame / 0x1000; i++)
	{
		kalloc_frame();
	}
}


pageframe_t kalloc_frame(void)
{
	uint32_t i = 0;
	while (frame_map[i/32] >> (i % 32) != 0)
	{		
		if (i == nframes - 1)
		{
			terminal_printhex32(i);
			kpanic("Ran out of page frames");
		}
		i++;
	}

	frame_map[i/32] |= 1 << (i % 32)/*just added this, could be wrong!!!*/;
	return i * 0x1000;
}


void kfree_frame(pageframe_t frame)
{
        frame /= 0x1000;

	if (frame >= nframes)
	{
		terminal_printhex32(nframes);
		kpanic("Tried to free a non-existant frame");
	}
	frame_map[frame/32] &= 0 << frame;
}
