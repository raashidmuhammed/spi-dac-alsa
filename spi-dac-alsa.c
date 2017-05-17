#include <linux/module.h>
#include <linux/of.h>
#include <linux/device.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>

#include <linux/spi/spi.h>

#define BUF_SIZE 16384

static char *id = SNDRV_DEFAULT_STR1;		/* ID for this card */

#ifdef CONFIG_OF
static const struct of_device_id spidac_dt_ids[] = {
	{ .compatible = "za-audio-p1" },
	{},
};
MODULE_DEVICE_TABLE(of, spidac_dt_ids);
#endif

struct snd_spi_dac {
	struct snd_card			*card;
	struct snd_pcm			*pcm;
	struct snd_pcm_substream	*substream;
	struct spi_device               *spi;
};

#define get_chip(card) ((struct snd_spi_dac *)card->private_data)

static struct snd_pcm_hardware snd_spi_dac_playback_hw = {
	.info		= SNDRV_PCM_INFO_INTERLEAVED |
			  SNDRV_PCM_INFO_BLOCK_TRANSFER,
	.formats	= SNDRV_PCM_FMTBIT_S16_BE,
	.rates		= SNDRV_PCM_RATE_CONTINUOUS,
	.rate_min	= 8000,  /* Replaced by chip->bitrate later. */
	.rate_max	= 50000, /* Replaced by chip->bitrate later. */
	.channels_min	= 1,
	.channels_max	= 2,
	.buffer_bytes_max = 64 * 1024 - 1,
	.period_bytes_min = 512,
	.period_bytes_max = 64 * 1024 - 1,
	.periods_min	= 4,
	.periods_max	= 1024,
};

static int snd_spi_write(struct spi_device *spi, const void *buf, size_t len)
{
	struct spi_transfer	t = {
			.tx_buf		= buf,
			.len		= len,
			.cs_change      = 0,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spi_sync(spi, &m);
}

static int snd_spi_dac_pcm_open(struct snd_pcm_substream *substream)
{
	printk("open function...\n");
	struct snd_spi_dac *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;

	printk("Max speed : %d", chip->spi->max_speed_hz);
//	runtime->hw = snd_spi_dac_playback_hw;


	return 0;
}

static int snd_spi_dac_pcm_close(struct snd_pcm_substream *substream)
{
	printk("close function...\n");
	return 0;
}

static int snd_spi_dac_pcm_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *hw_params)
{
	printk("hw_params function...\n");
	return 0;
}

static int snd_spi_dac_pcm_hw_free(struct snd_pcm_substream *substream)
{
	printk("hw_free function...\n");
	return 0;
}

static int snd_spi_dac_pcm_prepare(struct snd_pcm_substream *substream)
{
	printk("prepare function...\n");
	return 0;
}

static struct snd_pcm_ops spi_dac_playback_ops = {
	.open		= snd_spi_dac_pcm_open,
	.close		= snd_spi_dac_pcm_close,
	.hw_params	= snd_spi_dac_pcm_hw_params,
	.hw_free	= snd_spi_dac_pcm_hw_free,
	.prepare	= snd_spi_dac_pcm_prepare,
};

static int snd_spi_dac_dev_free(struct snd_device *device)
{
	return 0;
}


static int snd_spi_probe(struct spi_device *spi)
{
	printk("Entering probe...\n");
	struct snd_card *card;
	struct snd_pcm *pcm;
	struct snd_spi_dac *chip;
	static struct snd_device_ops ops = {
		.dev_free	= snd_spi_dac_dev_free,
	};


	int status = 0;

	int retval;

	retval = snd_card_new(&spi->dev, -1, id, THIS_MODULE,
			      sizeof(struct snd_spi_dac), &card);
	if (retval < 0) {
		printk("Error creating new sound card.\n");
		return -1;
	}

	chip = get_chip(card);
	chip->card = card;
	chip->spi = spi;

	printk("New PCM ...\n");
	retval = snd_pcm_new(chip->card, chip->card->shortname, 0, 1, 0, &pcm);
	if (retval < 0)	{
		printk("Adding new PCM device failed.\n");
		return retval;
	}

	pcm->private_data = chip;
	pcm->info_flags = SNDRV_PCM_INFO_BLOCK_TRANSFER;
	strcpy(pcm->name, "spi-dac-pcm");
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &spi_dac_playback_ops);

	/* retval = snd_pcm_lib_preallocate_pages_for_a ll(chip->pcm, */
	/* 		SNDRV_DMA_TYPE_CONTINUOUS, &chip->spi->dev, */
	/* 		64 * 1024, 64 * 1024); */
	/* if (retval) { */
	/* 	printk("Malloc failed...\n"); */
	/* 	return -1; */
	/* } */

	retval = snd_device_new(card, SNDRV_DEV_LOWLEVEL, chip, &ops);
	if (retval) {
		printk("Add new sound device failed.\n");
		return -1;
	}

	strcpy(card->driver, "spi_dac");
	printk("Register Sound card.\n");
	retval = snd_card_register(card);
	if (retval) {
		printk("Sound card registration failed.\n");
		snd_card_free(card);
		return -1;
	}

	dev_set_drvdata(&spi->dev, card);

	return status;
}

static int snd_spi_remove(struct spi_device *spi)
{
	struct snd_card *card = dev_get_drvdata(&spi->dev);

	snd_card_free(card);

	return 0;
}

static struct spi_driver spi_dac_driver = {
	.driver		= {
		.name	= "vybrid_spi",
		.of_match_table = of_match_ptr(spidac_dt_ids),
	},
	.probe		= snd_spi_probe,
	.remove		= snd_spi_remove,
};

module_spi_driver(spi_dac_driver);

MODULE_AUTHOR("Raashid <raashidmuhammed@zilogic.com>");
MODULE_DESCRIPTION("ALSA Sound driver for SPI DAC");
MODULE_LICENSE("GPL");
